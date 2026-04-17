# NEO — Tech Stack Document
> Version: 1.0 | Status: Finalized for v1 | Last Updated: April 2026

---

## 1. IoT / Firmware Stack

> Rule: ESP8266 is weak (~80KB RAM, 80MHz, single analog pin). Keep it lean. Arduino C++ is the right call.

### 1.1 Language & IDE

| Tech | Choice | Why |
|------|--------|-----|
| Language | **Arduino C++ (C++11)** | Native ESP8266 support, massive community, stable libs |
| IDE | **Arduino IDE 2.x** | Simple, board support built-in |
| Board Package | **ESP8266 Community (esp8266/Arduino)** | Official ESP8266 support |

> ✅ Yes, Arduino language (C++ with Arduino framework) is correct for ESP8266. It's the most documented and supported path. Avoid MicroPython — it's too slow for real-time RC522 RFID polling and uses more RAM.

### 1.2 Libraries

| Library | Purpose | Install via |
|---------|---------|-------------|
| `PubSubClient` by Nick O'Leary | MQTT client for ESP8266 | Arduino Library Manager |
| `MFRC522` by miguelbalboa | RC522 RFID reader | Arduino Library Manager |
| `ArduinoJson` (v6) | JSON serialize/deserialize | Arduino Library Manager |
| `ESP8266WiFi` | WiFi connection | Built-in (ESP8266 core) |
| `ESP8266HTTPClient` | HTTP calls if needed | Built-in (ESP8266 core) |

### 1.3 Hardware Components

**Have:**
| Component | Notes |
|-----------|-------|
| ESP8266 NodeMCU v3 | Main MCU. 3.3V logic. 1x analog pin (A0). |
| RC522 RFID Module | SPI. 3.3V. For attendance. |
| RFID Tags/Cards | UID-based identification. |

**To Buy (India — available on Robu.in / Zbotic.in / Amazon India):**
| Component | Recommended Spec | Est. Price (₹) |
|-----------|-----------------|----------------|
| Capacitive Soil Moisture Sensor v1.2 | Analog output, 3.3–5V, corrosion resistant | ₹80–150 |
| Micro Submersible Water Pump | 3–5V DC, low current (<500mA) | ₹80–120 |
| 1-Channel 5V Relay Module | Controls pump via GPIO signal | ₹40–80 |
| Jumper Wires (M-M, M-F) | For module connections | ₹50 |
| Small container for water reservoir | Manual DIY setup | — |

> ⚠️ Buy **capacitive** sensor, NOT resistive. Resistive sensors corrode in soil within weeks. Capacitive ones last years.

---

## 2. Web Stack

### 2.1 Frontend

| Tech | Choice | Why |
|------|--------|-----|
| Framework | **SvelteKit** | Compile-time, no virtual DOM, fastest bundle size, excellent for real-time dashboards. Cleaner than Next.js for this use case. |
| Language | **TypeScript** | Type safety, better DX, catches bugs early |
| Styling | **Tailwind CSS v4** | Utility-first, fast to build, looks good |
| UI Components | **shadcn-svelte** | Headless, accessible, customizable components. No bloat. |
| Charts | **Chart.js** (via `svelte-chartjs`) | Lightweight, great line/bar charts for sensor data |
| Real-time | **Native WebSocket API** (Svelte store) | Simple WS listener updating reactive stores |
| Icons | **Lucide Svelte** | Clean, consistent icon set |
| HTTP Client | **fetch API** (native, wrapped in `lib/api.ts`) | No need for Axios in SvelteKit |

### 2.2 Backend

| Tech | Choice | Why |
|------|--------|-----|
| Runtime | **Node.js 20 LTS** | Stable, great ecosystem for MQTT + WS + REST |
| Framework | **Fastify** | 2x faster than Express, TypeScript-native, plugin architecture, perfect for IoT APIs |
| Language | **TypeScript** | Consistent with frontend |
| ORM | **Drizzle ORM** | TypeScript-first, lightweight, SQL-like syntax, no magic |
| Validation | **Zod** | Schema validation for API inputs |
| MQTT Client | **mqtt.js (MQTT.js)** | Standard Node MQTT client, works with Mosquitto |
| WebSocket | **@fastify/websocket** | Official Fastify WS plugin |
| Auth | **JWT** via `@fastify/jwt` | Stateless, works well for single-user dashboard |
| Package Manager | **pnpm** | Fast, disk-efficient |

### 2.3 Database

| Tech | Choice | Why |
|------|--------|-----|
| Database | **PostgreSQL** | Reliable, free on Supabase/Railway, perfect for time-series + relational data |
| Hosting | **Supabase** (free tier) | 500MB free, hosted Postgres, built-in auth option if needed |
| Migrations | **Drizzle Kit** | Schema migrations that pair with Drizzle ORM |

### 2.4 MQTT Broker

| Tech | Choice | Why |
|------|--------|-----|
| Broker | **Mosquitto** (self-hosted via Docker) | Free, lightweight, standard, easy to configure |
| Hosting | **Railway.app** (Docker deploy) | Free tier available, simple deployment |
| Alternative | **HiveMQ Cloud** (free tier) | 100 concurrent connections free — zero-ops option |

### 2.5 Authentication

| Tech | Choice | Why |
|------|--------|-----|
| Strategy | **JWT (access + refresh tokens)** | Stateless, simple for single-user dashboard |
| Library | `@fastify/jwt` | Native Fastify integration |
| Storage (client) | `httpOnly cookie` | Secure, no XSS exposure |
| Future expansion | Can swap to **Lucia Auth** or **Better Auth** for multi-user | — |

### 2.6 Hosting & Deployment

| Service | What for | Cost |
|---------|----------|------|
| **Vercel** | SvelteKit frontend | Free |
| **Railway.app** | Fastify backend + Mosquitto broker | Free tier ($5/mo if needed) |
| **Supabase** | PostgreSQL database | Free (500MB) |

---

## 3. Development Tools

| Tool | Purpose |
|------|---------|
| VS Code | Primary editor |
| Arduino IDE 2.x | Firmware flashing |
| Postman / Bruno | API testing |
| TablePlus | PostgreSQL GUI |
| MQTT Explorer | Debug MQTT topics/messages |
| Git + GitHub | Version control |
| Docker (optional) | Run Mosquitto locally during dev |

---

## 4. Tech Decision Summary

```
IoT:    Arduino C++ + PubSubClient + ArduinoJson + MFRC522
Web:    SvelteKit + TypeScript + Tailwind + shadcn-svelte
API:    Fastify + TypeScript + Drizzle ORM + Zod
DB:     PostgreSQL (Supabase)
MQTT:   Mosquitto (Docker on Railway)
Auth:   JWT (httpOnly cookies)
Host:   Vercel (FE) + Railway (BE) + Supabase (DB)
```

---

## 5. What to Avoid (and Why)

| Avoid | Reason |
|-------|--------|
| MicroPython on ESP8266 | Too slow for real-time RFID polling, higher RAM usage than C++ |
| Next.js | Heavier than needed, React overhead unnecessary for this dashboard |
| MongoDB | Overkill for this structured time-series + relational data |
| Firebase Realtime DB | Vendor lock-in, harder to query, overkill |
| Resistive moisture sensor | Corrodes in soil quickly |
| Express.js | Too slow compared to Fastify, no schema validation built-in |