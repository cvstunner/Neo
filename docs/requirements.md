# NEO — Project Requirements Document
> Version: 1.0 | Status: Draft | Author: You | Last Updated: April 2026

---

## 1. Project Overview

**NEO** is a personal IoT assistant device placed near a Snake Plant. It handles daily physical and data tasks — plant care automation, attendance tracking via RFID, and a web dashboard for monitoring and control. The device is modular, allowing sensors/actuators to be added or removed as needed.

---

## 2. Web Requirements

### 2.1 Functional Requirements

| ID | Requirement | Priority |
|----|-------------|----------|
| W-001 | Real-time dashboard displaying plant moisture level | High |
| W-002 | Live pump status (ON/OFF) and manual pump trigger | High |
| W-003 | Attendance log — view scanned RFID entries with timestamp and user name | High |
| W-004 | Historical moisture chart (line chart, selectable time ranges) | High |
| W-005 | Notifications/alerts when moisture drops below threshold | Medium |
| W-006 | Manual watering override (trigger pump from web) | Medium |
| W-007 | User authentication (login to access dashboard) | High |
| W-008 | Device status panel (online/offline, last seen, firmware version) | Medium |
| W-009 | Attendance summary — daily/weekly view, total count per user | Medium |
| W-010 | RFID user management — link UID to a name/profile | Medium |

### 2.2 Non-Functional Requirements

- Page load under 1.5s on desktop and mobile
- Responsive UI (mobile-first design)
- Real-time data updates via WebSocket (no manual refresh)
- Secure API endpoints (JWT auth)
- Should look clean, modern, and visually polished

---

## 3. Device (IoT) Requirements

### 3.1 Core Hardware

| Component | Purpose | Status |
|-----------|---------|--------|
| ESP8266 (NodeMCU) | Main MCU — WiFi, logic, MQTT | Have |
| RFID RC522 Module | Scan RFID tags for attendance | Have |
| RFID Tags/Cards | Individual user identity tokens | Have |

### 3.2 Module System (Removable/Pluggable)

**Module A — Moisture Sensor**
- Capacitive Soil Moisture Sensor v1.2 (analog)
- Reads moisture % via A0 pin
- Reports to server via MQTT every N minutes
- Status: To Buy (available on Robu.in / Zbotic.in ~₹80–150)

**Module B — Auto Water Pump**
- Micro submersible water pump (3–5V)
- Controlled via relay module
- Triggered automatically when moisture < threshold OR manually from web
- Status: To Buy

**Module C — RFID Scanner (already have)**
- RC522 RFID reader
- Reads tag UID on scan
- Publishes event to server via MQTT
- Status: Have

### 3.3 Functional Requirements

| ID | Requirement | Priority |
|----|-------------|----------|
| D-001 | Read soil moisture every 10 minutes and publish to MQTT broker | High |
| D-002 | Auto-trigger pump when moisture < configurable threshold | High |
| D-003 | Accept remote pump trigger command from server via MQTT | High |
| D-004 | Scan RFID tag and publish UID + timestamp to MQTT | High |
| D-005 | Deep sleep between readings to save power | Medium |
| D-006 | WiFi reconnect on disconnect with retry logic | High |
| D-007 | Configurable parameters via code (thresholds, sleep time, broker URL) | Medium |
| D-008 | LED status indicator (connecting, connected, pump active, scan success) | Low |
| D-009 | OTA (Over-the-Air) firmware updates via WiFi | Future |

### 3.4 Non-Functional Requirements

- Runs on 3.3V logic (ESP8266 native)
- MQTT payload in JSON format
- Must reconnect automatically after WiFi drop
- Firmware written in Arduino C++ (Arduino IDE compatible)
- Memory-efficient — ESP8266 has only ~80KB RAM

---

## 4. Future Requirements (Planned, Not Current Sprint)

> These are tracked here to ensure architecture decisions today don't block future expansion.

| ID | Requirement | Category |
|----|-------------|----------|
| F-001 | Upgrade MCU to ESP32 for more GPIO, better ADC, dual-core | IoT |
| F-002 | Add DHT22 temperature + humidity sensor | IoT |
| F-003 | Add light sensor (LDR or BH1750) | IoT |
| F-004 | Add small OLED display on device for local status | IoT |
| F-005 | AI-powered watering recommendations based on historical data | Web/AI |
| F-006 | Plant health analysis with trend detection (AI) | Web/AI |
| F-007 | Push notifications (mobile / browser) for alerts | Web |
| F-008 | Multi-plant support (multiple sensor nodes) | IoT/Web |
| F-009 | Mobile app (PWA or native) | Mobile |
| F-010 | Voice assistant integration (remind me to check plant) | AI |
| F-011 | Complex analytics dashboard — weekly trends, heatmaps | Web |
| F-012 | External API for family/housemates to see attendance | Web |

---

## 5. Constraints

- Budget: Minimal (use existing hardware, buy cheap sensors ~₹500–1000 total)
- MCU: ESP8266 for v1 — weak processor, limited RAM, single analog pin (A0)
- Internet: Must work on home WiFi only (no SIM/cellular needed for v1)
- Hosting: Free tier preferred for server/DB (Railway, Render, Supabase, etc.)
- Developer: Solo developer — keep stack simple and fast to build

---

## 6. Assumptions

- Device is always plugged into power (no battery required for v1)
- WiFi is stable at device placement location
- One ESP8266 handles all three modules (moisture + pump + RFID) in v1
- Web dashboard is private (single user or small household)