# NEO — Architecture Document
> Version: 1.0 | Status: Draft | Last Updated: April 2026

---

## 1. System Overview

```
┌─────────────────────────────────────────────────────────────┐
│                        NEO SYSTEM                           │
│                                                             │
│  ┌───────────────┐         ┌────────────────────────────┐  │
│  │  IoT Device   │ ──MQTT──▶    Backend Server (Node)   │  │
│  │  (ESP8266)    │◀──MQTT── │  - REST API               │  │
│  └───────────────┘         │  - MQTT Broker (Mosquitto) │  │
│                            │  - WebSocket               │  │
│                            └────────────┬───────────────┘  │
│                                         │                   │
│                                         │ DB                │
│                                    ┌────▼────┐              │
│                                    │ Postgres │              │
│                                    └────┬────┘              │
│                                         │                   │
│                                    ┌────▼────────────────┐  │
│                                    │  Web Client         │  │
│                                    │  (SvelteKit)        │  │
│                                    └─────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

Data Flow:
1. ESP8266 reads sensors → publishes JSON to MQTT broker
2. Backend subscribes to MQTT → processes data → stores in PostgreSQL
3. Backend pushes real-time update to Web via WebSocket
4. Web Client renders data live on dashboard
5. Web Client sends pump command → API → MQTT publish → ESP8266 receives → triggers pump

---

## 2. IoT Architecture

### 2.1 Top-Level Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    ESP8266 (NodeMCU)                     │
│                                                         │
│  ┌──────────────┐   ┌──────────────┐   ┌────────────┐  │
│  │  Moisture    │   │   RFID       │   │   Relay    │  │
│  │  Module (A0) │   │   Module     │   │   Module   │  │
│  │              │   │   (SPI)      │   │   (GPIO)   │  │
│  └──────┬───────┘   └──────┬───────┘   └─────┬──────┘  │
│         │                  │                  │         │
│  ┌──────▼──────────────────▼──────────────────▼──────┐  │
│  │              Main Firmware Loop                   │  │
│  │  - WiFi Manager     - MQTT Client                 │  │
│  │  - Sensor Reader    - Command Handler             │  │
│  │  - JSON Builder     - LED Status                  │  │
│  └───────────────────────────┬───────────────────────┘  │
│                              │ MQTT over TCP/WiFi         │
└──────────────────────────────┼─────────────────────────-─┘
                               │
                         MQTT Broker
```

### 2.2 Low-Level Architecture (Firmware)

**Modules / Files:**
```
firmware/
├── neo.ino                  ← Main entry: setup() + loop()
├── config.h                 ← WiFi creds, MQTT broker, thresholds (gitignored)
├── mqtt_client.cpp/.h       ← MQTT connect, publish, subscribe, callback
├── moisture_sensor.cpp/.h   ← Read A0, calibrate to %, smoothing
├── rfid_scanner.cpp/.h      ← RC522 init, read tag UID, debounce
├── pump_controller.cpp/.h   ← Relay GPIO control, auto + manual mode
└── led_status.cpp/.h        ← LED patterns for each state
```

**Firmware State Machine:**
```
BOOT
  │
  ▼
WiFi Connect (retry 10x, blink LED fast)
  │
  ▼
MQTT Connect (retry 5x)
  │
  ▼
IDLE LOOP
  ├── Every 10 min → Read moisture → Publish neo/sensor/moisture
  ├── Continuous  → Poll RFID → On scan: Publish neo/rfid/scan
  ├── Continuous  → Check MQTT → On cmd neo/pump/control → Toggle pump
  └── Auto logic  → If moisture < threshold AND pump not running → Trigger pump
```

**MQTT Topics (Device Side):**
```
PUBLISH  neo/sensor/moisture     { "value": 42, "unit": "%" }
PUBLISH  neo/rfid/scan           { "uid": "A1B2C3D4", "timestamp": 1234567890 }
PUBLISH  neo/device/status       { "online": true, "ip": "192.168.x.x", "rssi": -60 }
SUBSCRIBE neo/pump/control       { "action": "on" | "off" }
```

**Pin Mapping (NodeMCU ESP8266):**
```
A0        → Moisture Sensor AOUT
D1 (GPIO5)  → RFID SCK
D2 (GPIO4)  → RFID MISO (SDA/SS)
D5 (GPIO14) → RFID MOSI
D6 (GPIO12) → RFID SCK
D7 (GPIO13) → RFID RST
D0 (GPIO16) → Relay IN (Pump)
D4 (GPIO2)  → Onboard LED (status)
```

### 2.3 Power & Optimization Strategy

ESP8266 is resource-constrained (~80KB RAM, single-core 80MHz). Key strategies:

- Use `millis()` timers instead of `delay()` to avoid blocking the loop
- Use lightweight `PubSubClient` library for MQTT (not heavy WebSocket libraries)
- Keep JSON payloads small — no unnecessary fields
- Avoid String class — use `char[]` buffers to reduce heap fragmentation
- RFID polling is interrupt-friendly — use `MFRC522::PICC_IsNewCardPresent()` in loop
- Deep sleep can be added later when on battery (v2 with ESP32)

---

## 3. Web Architecture

### 3.1 Top-Level Architecture

```
┌────────────────────────────────────────────────────────┐
│                     WEB LAYER                          │
│                                                        │
│  ┌──────────────────────────────┐                      │
│  │     SvelteKit (Client)       │                      │
│  │  - Dashboard page            │                      │
│  │  - Attendance page           │                      │
│  │  - Settings page             │                      │
│  │  - Auth pages (login)        │                      │
│  │  - WebSocket listener        │                      │
│  └──────────────┬───────────────┘                      │
│                 │ HTTP REST + WS                        │
│  ┌──────────────▼───────────────┐                      │
│  │     Fastify (API Server)     │                      │
│  │  - REST Routes (/api/*)      │                      │
│  │  - JWT Auth Middleware       │                      │
│  │  - WebSocket Server          │                      │
│  │  - MQTT Subscriber           │                      │
│  │  - DB Access (Drizzle ORM)   │                      │
│  └──────────────┬───────────────┘                      │
│                 │                                       │
│  ┌──────────────▼───────────────┐                      │
│  │     PostgreSQL (DB)          │                      │
│  │  - moisture_readings         │                      │
│  │  - attendance_logs           │                      │
│  │  - rfid_users                │                      │
│  │  - pump_events               │                      │
│  │  - users (auth)              │                      │
│  └──────────────────────────────┘                      │
└────────────────────────────────────────────────────────┘
```

### 3.2 Low-Level Architecture (Backend)

**Folder Structure:**
```
backend/
├── src/
│   ├── index.ts                  ← Server entry, register plugins
│   ├── config.ts                 ← ENV vars (dotenv)
│   ├── db/
│   │   ├── schema.ts             ← Drizzle table definitions
│   │   └── index.ts              ← DB connection (pg pool)
│   ├── mqtt/
│   │   ├── client.ts             ← Connect to Mosquitto, subscribe to topics
│   │   └── handlers.ts           ← Handle each MQTT message type
│   ├── routes/
│   │   ├── auth.ts               ← POST /api/auth/login, /logout, /refresh
│   │   ├── moisture.ts           ← GET /api/moisture (history, latest)
│   │   ├── attendance.ts         ← GET /api/attendance, GET /api/users
│   │   ├── pump.ts               ← POST /api/pump/control, GET /api/pump/status
│   │   └── device.ts             ← GET /api/device/status
│   ├── websocket/
│   │   └── server.ts             ← Broadcast real-time events to clients
│   └── middleware/
│       └── auth.ts               ← JWT verify hook
├── .env                          ← Secrets (gitignored)
└── package.json
```

**Request Lifecycle:**
```
Client (SvelteKit)
    │
    ├─ REST: GET /api/moisture
    │         → Auth middleware → Route handler → Drizzle query → JSON response
    │
    ├─ WS: Connect ws://server/ws
    │         → Client registered in ws pool
    │         → On MQTT event → Server broadcasts to all WS clients
    │
    └─ POST /api/pump/control { action: "on" }
              → Auth → Validate → Publish to MQTT → Respond 200
```

### 3.3 Low-Level Architecture (Frontend)

**Folder Structure:**
```
frontend/
├── src/
│   ├── routes/
│   │   ├── +layout.svelte        ← Global layout, navbar, auth check
│   │   ├── +page.svelte          ← Dashboard (moisture, pump, device status)
│   │   ├── attendance/
│   │   │   └── +page.svelte      ← Attendance log table
│   │   ├── settings/
│   │   │   └── +page.svelte      ← Threshold config, RFID user management
│   │   └── login/
│   │       └── +page.svelte      ← Login form
│   ├── lib/
│   │   ├── api.ts                ← Fetch wrappers for all API calls
│   │   ├── ws.ts                 ← WebSocket client + store update
│   │   ├── stores/
│   │   │   ├── moisture.ts       ← Svelte writable store for live moisture data
│   │   │   ├── attendance.ts     ← Store for attendance logs
│   │   │   └── device.ts         ← Store for device status
│   │   └── components/
│   │       ├── MoistureGauge.svelte    ← Circular or bar gauge
│   │       ├── MoistureChart.svelte    ← Line chart (Chart.js or Recharts)
│   │       ├── AttendanceTable.svelte  ← Sortable log table
│   │       ├── PumpControl.svelte      ← Toggle with confirmation
│   │       └── DeviceStatusCard.svelte ← Online/offline badge
│   └── app.css                   ← Global Tailwind base + custom tokens
├── static/
└── svelte.config.js
```

---

## 4. Infrastructure

```
┌─────────────────────────────────────────┐
│              Deployment                 │
│                                         │
│  Backend + MQTT Broker                  │
│  → Railway.app (free tier, Docker)      │
│    OR Render.com (free tier)            │
│                                         │
│  Frontend (SvelteKit)                   │
│  → Vercel (free tier, auto-deploy)      │
│                                         │
│  Database (PostgreSQL)                  │
│  → Supabase (free tier, 500MB)          │
│    OR Railway Postgres plugin           │
│                                         │
│  MQTT Broker                            │
│  → Mosquitto in Docker on Railway       │
│    OR HiveMQ Cloud (free 100 sessions)  │
└─────────────────────────────────────────┘
```