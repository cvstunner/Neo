# NEO — Server & Client Documentation
> Version: 1.0 | Last Updated: April 2026

---

# PART 1: SERVER DOCUMENTATION

## 1. Overview

The NEO backend is a **Fastify + TypeScript** server that:
- Exposes REST API for the web client
- Maintains a WebSocket server for real-time push
- Subscribes to MQTT broker topics from the ESP8266
- Persists all data to PostgreSQL via Drizzle ORM

---

## 2. Environment Variables

Create `.env` in the backend root:

```env
# Server
PORT=3000
NODE_ENV=development

# Database
DATABASE_URL=postgresql://user:password@host:5432/neo_db

# JWT
JWT_SECRET=your-super-secret-key-change-this
JWT_EXPIRES_IN=15m
JWT_REFRESH_EXPIRES_IN=7d

# MQTT
MQTT_BROKER_URL=mqtt://localhost:1883
MQTT_USERNAME=neo
MQTT_PASSWORD=neo-mqtt-password

# Admin (first-time setup)
ADMIN_USERNAME=admin
ADMIN_PASSWORD=changeme123
```

---

## 3. Server Setup & Bootstrap

```typescript
// src/index.ts
import Fastify from 'fastify';
import { fastifyJwt } from '@fastify/jwt';
import { fastifyWebsocket } from '@fastify/websocket';
import { fastifyCookie } from '@fastify/cookie';
import { db } from './db/index';
import { mqttClient } from './mqtt/client';

const app = Fastify({ logger: true });

// Plugins
app.register(fastifyJwt, { secret: process.env.JWT_SECRET! });
app.register(fastifyWebsocket);
app.register(fastifyCookie);

// Routes
app.register(authRoutes, { prefix: '/api/auth' });
app.register(moistureRoutes, { prefix: '/api/moisture' });
app.register(attendanceRoutes, { prefix: '/api/attendance' });
app.register(pumpRoutes, { prefix: '/api/pump' });
app.register(userRoutes, { prefix: '/api/users' });
app.register(deviceRoutes, { prefix: '/api/device' });
app.register(wsRoutes, { prefix: '/ws' });

// Start
app.listen({ port: Number(process.env.PORT) || 3000 });
mqttClient.connect();
```

---

## 4. MQTT Integration

```typescript
// src/mqtt/client.ts
import mqtt from 'mqtt';
import { handleMoisture, handleRfid, handleDeviceStatus } from './handlers';

const TOPICS = [
  'neo/sensor/moisture',
  'neo/rfid/scan',
  'neo/device/status',
];

export const mqttClient = {
  client: null as mqtt.MqttClient | null,

  connect() {
    this.client = mqtt.connect(process.env.MQTT_BROKER_URL!, {
      username: process.env.MQTT_USERNAME,
      password: process.env.MQTT_PASSWORD,
      reconnectPeriod: 5000,
    });

    this.client.on('connect', () => {
      console.log('MQTT connected');
      TOPICS.forEach(t => this.client!.subscribe(t));
    });

    this.client.on('message', (topic, payload) => {
      const data = JSON.parse(payload.toString());
      if (topic === 'neo/sensor/moisture') handleMoisture(data);
      if (topic === 'neo/rfid/scan') handleRfid(data);
      if (topic === 'neo/device/status') handleDeviceStatus(data);
    });
  },

  publish(topic: string, payload: object) {
    this.client?.publish(topic, JSON.stringify(payload));
  },
};
```

```typescript
// src/mqtt/handlers.ts
import { db } from '../db';
import { moistureReadings, attendanceLogs } from '../db/schema';
import { broadcast } from '../websocket/server';

export async function handleMoisture(data: { value: number; raw_value?: number }) {
  // Save to DB
  await db.insert(moistureReadings).values({ value: data.value, rawValue: data.raw_value });
  // Push to web clients
  broadcast({ type: 'moisture.update', value: data.value, timestamp: new Date() });
}

export async function handleRfid(data: { uid: string; timestamp: number }) {
  const user = await db.query.rfidUsers.findFirst({
    where: (u, { eq }) => eq(u.rfidUid, data.uid),
  });
  await db.insert(attendanceLogs).values({ rfidUid: data.uid, rfidUserId: user?.id });
  broadcast({ type: 'rfid.scan', uid: data.uid, user: user?.name ?? 'Unknown' });
}
```

---

## 5. WebSocket Server

```typescript
// src/websocket/server.ts
const clients = new Set<WebSocket>();

export function wsRoutes(app: FastifyInstance) {
  app.get('/ws', { websocket: true }, (socket) => {
    clients.add(socket);
    socket.on('close', () => clients.delete(socket));
  });
}

export function broadcast(payload: object) {
  const msg = JSON.stringify(payload);
  clients.forEach(ws => {
    if (ws.readyState === ws.OPEN) ws.send(msg);
  });
}
```

---

## 6. Auth Middleware

```typescript
// src/middleware/auth.ts
export async function requireAuth(request: FastifyRequest, reply: FastifyReply) {
  try {
    await request.jwtVerify();
  } catch {
    reply.code(401).send({ error: 'Unauthorized' });
  }
}
```

Usage in routes:
```typescript
app.get('/api/moisture/latest', { preHandler: [requireAuth] }, handler);
```

---

## 7. Running the Server

```bash
# Install
pnpm install

# Dev (with hot reload)
pnpm dev          # uses tsx watch

# Production
pnpm build        # tsc
pnpm start        # node dist/index.js
```

---
---

# PART 2: CLIENT DOCUMENTATION

## 1. Overview

The NEO frontend is a **SvelteKit + TypeScript** app. It connects to the backend via REST (initial data) and WebSocket (live updates). The UI is built with Tailwind CSS + shadcn-svelte components.

---

## 2. Project Structure

```
frontend/src/
├── routes/
│   ├── +layout.svelte          ← Nav, auth guard, WS init
│   ├── +page.svelte            ← Dashboard: moisture, pump, device
│   ├── attendance/+page.svelte ← Attendance log
│   ├── settings/+page.svelte   ← Thresholds, RFID user management
│   └── login/+page.svelte      ← Login form
├── lib/
│   ├── api.ts                  ← All fetch calls
│   ├── ws.ts                   ← WS connection + store dispatch
│   └── stores/
│       ├── moisture.ts
│       ├── attendance.ts
│       └── device.ts
```

---

## 3. API Client (`lib/api.ts`)

```typescript
const BASE = import.meta.env.VITE_API_URL ?? 'http://localhost:3000';

async function apiFetch<T>(path: string, init?: RequestInit): Promise<T> {
  const res = await fetch(`${BASE}${path}`, {
    credentials: 'include', // sends cookies
    headers: { 'Content-Type': 'application/json' },
    ...init,
  });
  if (!res.ok) throw await res.json();
  return res.json();
}

export const api = {
  moisture: {
    latest: () => apiFetch('/api/moisture/latest'),
    history: (range = '24h') => apiFetch(`/api/moisture/history?range=${range}`),
  },
  pump: {
    status: () => apiFetch('/api/pump/status'),
    control: (action: 'on' | 'off') =>
      apiFetch('/api/pump/control', { method: 'POST', body: JSON.stringify({ action }) }),
  },
  attendance: {
    list: (date?: string) => apiFetch(`/api/attendance${date ? `?date=${date}` : ''}`),
  },
  users: {
    list: () => apiFetch('/api/users'),
    create: (name: string, rfidUid: string) =>
      apiFetch('/api/users', { method: 'POST', body: JSON.stringify({ name, rfidUid }) }),
    delete: (id: number) => apiFetch(`/api/users/${id}`, { method: 'DELETE' }),
  },
  auth: {
    login: (username: string, password: string) =>
      apiFetch('/api/auth/login', { method: 'POST', body: JSON.stringify({ username, password }) }),
    logout: () => apiFetch('/api/auth/logout', { method: 'POST' }),
  },
};
```

---

## 4. WebSocket Client (`lib/ws.ts`)

```typescript
import { moistureStore } from './stores/moisture';
import { attendanceStore } from './stores/attendance';

let socket: WebSocket;

export function connectWS() {
  socket = new WebSocket(import.meta.env.VITE_WS_URL ?? 'ws://localhost:3000/ws');

  socket.onmessage = (event) => {
    const msg = JSON.parse(event.data);

    if (msg.type === 'moisture.update') {
      moistureStore.update(s => ({ ...s, current: msg.value, lastUpdated: msg.timestamp }));
    }

    if (msg.type === 'rfid.scan') {
      attendanceStore.update(s => ({
        ...s,
        logs: [{ uid: msg.uid, user: msg.user, timestamp: msg.timestamp }, ...s.logs],
      }));
    }
  };

  socket.onclose = () => {
    setTimeout(connectWS, 3000); // auto-reconnect
  };
}
```

---

## 5. Svelte Stores

```typescript
// stores/moisture.ts
import { writable } from 'svelte/store';

export const moistureStore = writable({
  current: 0,
  history: [] as { value: number; timestamp: string }[],
  lastUpdated: null as string | null,
});
```

---

## 6. Dashboard Page (`routes/+page.svelte`)

Key components rendered:
- `MoistureGauge` — circular gauge showing current moisture %
- `MoistureChart` — Chart.js line chart, time-range selector
- `PumpControl` — toggle button + confirmation modal
- `DeviceStatusCard` — online badge, last seen, IP, RSSI

```svelte
<script lang="ts">
  import { onMount } from 'svelte';
  import { api } from '$lib/api';
  import { connectWS } from '$lib/ws';
  import { moistureStore } from '$lib/stores/moisture';
  import MoistureGauge from '$lib/components/MoistureGauge.svelte';
  import MoistureChart from '$lib/components/MoistureChart.svelte';
  import PumpControl from '$lib/components/PumpControl.svelte';

  onMount(async () => {
    connectWS();
    const latest = await api.moisture.latest();
    moistureStore.update(s => ({ ...s, current: latest.value }));
  });
</script>

<div class="grid grid-cols-1 md:grid-cols-2 gap-6 p-6">
  <MoistureGauge value={$moistureStore.current} />
  <PumpControl />
  <MoistureChart history={$moistureStore.history} />
</div>
```

---

## 7. Environment Variables (Frontend)

```env
# .env
VITE_API_URL=http://localhost:3000
VITE_WS_URL=ws://localhost:3000/ws
```

---

## 8. Running the Client

```bash
# Install
pnpm install

# Dev
pnpm dev         # http://localhost:5173

# Build
pnpm build
pnpm preview
```