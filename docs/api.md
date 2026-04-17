# NEO — API Documentation
> Version: 1.0 | Base URL: `http://localhost:3000` (dev) | `https://neo-api.railway.app` (prod)

All endpoints require `Authorization: Bearer <token>` unless marked **[PUBLIC]**.

---

## 1. Authentication

### POST /api/auth/login `[PUBLIC]`
Login and receive JWT tokens.

**Request:**
```json
{
  "username": "admin",
  "password": "your-password"
}
```

**Response 200:**
```json
{
  "accessToken": "eyJ...",
  "user": { "id": 1, "username": "admin" }
}
```
Access token is also set as `httpOnly` cookie.

**Response 401:**
```json
{ "error": "Invalid credentials" }
```

---

### POST /api/auth/logout
Clear auth cookie and invalidate session.

**Response 200:**
```json
{ "message": "Logged out" }
```

---

### GET /api/auth/me
Get current authenticated user info.

**Response 200:**
```json
{
  "id": 1,
  "username": "admin",
  "createdAt": "2026-01-01T00:00:00Z"
}
```

---

## 2. Moisture / Plant

### GET /api/moisture/latest
Get the most recent moisture reading.

**Response 200:**
```json
{
  "id": 123,
  "value": 42,
  "unit": "percent",
  "timestamp": "2026-04-10T08:30:00Z"
}
```

---

### GET /api/moisture/history
Get historical moisture readings.

**Query Params:**
| Param | Type | Default | Description |
|-------|------|---------|-------------|
| `range` | `"1h" \| "6h" \| "24h" \| "7d" \| "30d"` | `"24h"` | Time range |
| `limit` | number | 100 | Max records |

**Response 200:**
```json
{
  "data": [
    { "value": 42, "timestamp": "2026-04-10T08:30:00Z" },
    { "value": 38, "timestamp": "2026-04-10T08:20:00Z" }
  ],
  "count": 2,
  "range": "24h"
}
```

---

## 3. Pump Control

### GET /api/pump/status
Get current pump state.

**Response 200:**
```json
{
  "status": "off",
  "lastTriggeredAt": "2026-04-10T07:00:00Z",
  "lastTriggeredBy": "auto"
}
```
`status`: `"on" | "off"`
`lastTriggeredBy`: `"auto" | "manual"`

---

### POST /api/pump/control
Trigger or stop the pump manually.

**Request:**
```json
{
  "action": "on"
}
```
`action`: `"on" | "off"`

**Response 200:**
```json
{
  "success": true,
  "action": "on",
  "publishedTo": "neo/pump/control"
}
```

**Response 400:**
```json
{ "error": "Invalid action. Use 'on' or 'off'" }
```

---

### GET /api/pump/history
Get past pump activation events.

**Query Params:**
| Param | Type | Default |
|-------|------|---------|
| `limit` | number | 20 |
| `offset` | number | 0 |

**Response 200:**
```json
{
  "data": [
    {
      "id": 5,
      "action": "on",
      "triggeredBy": "auto",
      "moistureAtTrigger": 18,
      "timestamp": "2026-04-10T07:00:00Z"
    }
  ],
  "total": 1
}
```

---

## 4. Attendance / RFID

### GET /api/attendance
Get attendance log entries.

**Query Params:**
| Param | Type | Default |
|-------|------|---------|
| `date` | `YYYY-MM-DD` | today |
| `userId` | number | all |
| `limit` | number | 50 |
| `offset` | number | 0 |

**Response 200:**
```json
{
  "data": [
    {
      "id": 10,
      "uid": "A1B2C3D4",
      "user": { "id": 2, "name": "Rushil", "rfidUid": "A1B2C3D4" },
      "timestamp": "2026-04-10T09:15:00Z"
    }
  ],
  "total": 1
}
```

---

### GET /api/attendance/summary
Get daily attendance count per user.

**Query Params:**
| Param | Type |
|-------|------|
| `startDate` | `YYYY-MM-DD` |
| `endDate` | `YYYY-MM-DD` |

**Response 200:**
```json
{
  "data": [
    { "user": "Rushil", "date": "2026-04-10", "count": 3 }
  ]
}
```

---

## 5. RFID Users

### GET /api/users
Get all registered RFID users.

**Response 200:**
```json
{
  "data": [
    { "id": 1, "name": "Rushil", "rfidUid": "A1B2C3D4", "createdAt": "2026-01-01T00:00:00Z" }
  ]
}
```

---

### POST /api/users
Register a new RFID user.

**Request:**
```json
{
  "name": "Mom",
  "rfidUid": "F4E3D2C1"
}
```

**Response 201:**
```json
{
  "id": 2,
  "name": "Mom",
  "rfidUid": "F4E3D2C1"
}
```

**Response 409:**
```json
{ "error": "UID already registered" }
```

---

### DELETE /api/users/:id
Remove a registered RFID user.

**Response 200:**
```json
{ "success": true }
```

---

## 6. Device Status

### GET /api/device/status
Get NEO device status.

**Response 200:**
```json
{
  "online": true,
  "lastSeen": "2026-04-10T09:30:00Z",
  "ip": "192.168.1.42",
  "rssi": -62,
  "firmwareVersion": "1.0.0"
}
```

---

## 7. WebSocket Events

**Connect:** `ws://localhost:3000/ws`

**Authenticate:** Send after connect:
```json
{ "type": "auth", "token": "eyJ..." }
```

**Server-sent Events:**

| Event type | Payload | Description |
|------------|---------|-------------|
| `moisture.update` | `{ value: 42, timestamp: "..." }` | New moisture reading |
| `rfid.scan` | `{ uid: "A1B2C3D4", user: "Rushil", timestamp: "..." }` | RFID tag scanned |
| `pump.status` | `{ status: "on", triggeredBy: "auto" }` | Pump state changed |
| `device.status` | `{ online: true, lastSeen: "..." }` | Device connected/disconnected |

---

## 8. Error Responses

All errors follow this format:
```json
{
  "error": "Human-readable message",
  "code": "MACHINE_CODE",
  "statusCode": 400
}
```

Common HTTP codes:
| Code | Meaning |
|------|---------|
| 400 | Bad request / validation error |
| 401 | Not authenticated |
| 403 | Forbidden |
| 404 | Resource not found |
| 409 | Conflict (duplicate) |
| 500 | Internal server error |