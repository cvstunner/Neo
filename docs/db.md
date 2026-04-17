# NEO — Database Documentation
> Version: 1.0 | DB: PostgreSQL | ORM: Drizzle ORM | Last Updated: April 2026

---

## 1. Overview

PostgreSQL is used for all persistent data in NEO. The schema is designed for:
- Time-series sensor readings (moisture)
- Attendance event logs
- RFID user registry
- Pump event history
- Auth user table

Hosted on **Supabase** (free tier, 500MB).

---

## 2. Schema

### 2.1 `users` (Auth Users)

Login accounts for the web dashboard.

```sql
CREATE TABLE users (
  id          SERIAL PRIMARY KEY,
  username    VARCHAR(50) UNIQUE NOT NULL,
  password    VARCHAR(255) NOT NULL,  -- bcrypt hashed
  created_at  TIMESTAMPTZ DEFAULT NOW()
);
```

| Column | Type | Notes |
|--------|------|-------|
| id | SERIAL | Primary key |
| username | VARCHAR(50) | Unique login name |
| password | VARCHAR(255) | bcrypt hash |
| created_at | TIMESTAMPTZ | Auto set |

---

### 2.2 `rfid_users` (Registered RFID Identities)

Maps RFID tag UIDs to real names.

```sql
CREATE TABLE rfid_users (
  id          SERIAL PRIMARY KEY,
  name        VARCHAR(100) NOT NULL,
  rfid_uid    VARCHAR(20) UNIQUE NOT NULL,
  created_at  TIMESTAMPTZ DEFAULT NOW()
);
```

| Column | Type | Notes |
|--------|------|-------|
| id | SERIAL | Primary key |
| name | VARCHAR(100) | Person's name |
| rfid_uid | VARCHAR(20) | Unique tag UID (hex string e.g. "A1B2C3D4") |
| created_at | TIMESTAMPTZ | When registered |

---

### 2.3 `moisture_readings`

Time-series soil moisture data from ESP8266.

```sql
CREATE TABLE moisture_readings (
  id          SERIAL PRIMARY KEY,
  value       SMALLINT NOT NULL,       -- 0-100 percent
  raw_value   SMALLINT,                -- Raw ADC value (0-1023) for calibration
  timestamp   TIMESTAMPTZ DEFAULT NOW()
);

CREATE INDEX idx_moisture_timestamp ON moisture_readings(timestamp DESC);
```

| Column | Type | Notes |
|--------|------|-------|
| id | SERIAL | Primary key |
| value | SMALLINT | Moisture % (0–100) |
| raw_value | SMALLINT | Raw ADC (optional, for calibration review) |
| timestamp | TIMESTAMPTZ | Reading time |

> Index on `timestamp DESC` — all queries are time-range based, this keeps them fast.

---

### 2.4 `attendance_logs`

Log of every RFID scan event.

```sql
CREATE TABLE attendance_logs (
  id            SERIAL PRIMARY KEY,
  rfid_uid      VARCHAR(20) NOT NULL,
  rfid_user_id  INTEGER REFERENCES rfid_users(id) ON DELETE SET NULL,
  timestamp     TIMESTAMPTZ DEFAULT NOW()
);

CREATE INDEX idx_attendance_timestamp ON attendance_logs(timestamp DESC);
CREATE INDEX idx_attendance_user ON attendance_logs(rfid_user_id);
```

| Column | Type | Notes |
|--------|------|-------|
| id | SERIAL | Primary key |
| rfid_uid | VARCHAR(20) | Raw UID from tag (stored even if user not registered) |
| rfid_user_id | INTEGER FK | Link to rfid_users (nullable — unknown tags still logged) |
| timestamp | TIMESTAMPTZ | When scan happened |

---

### 2.5 `pump_events`

Log of every pump activation/deactivation.

```sql
CREATE TABLE pump_events (
  id                  SERIAL PRIMARY KEY,
  action              VARCHAR(10) NOT NULL,   -- 'on' or 'off'
  triggered_by        VARCHAR(10) NOT NULL,   -- 'auto' or 'manual'
  moisture_at_trigger SMALLINT,               -- moisture % when triggered
  timestamp           TIMESTAMPTZ DEFAULT NOW()
);
```

| Column | Type | Notes |
|--------|------|-------|
| id | SERIAL | Primary key |
| action | VARCHAR(10) | `'on'` or `'off'` |
| triggered_by | VARCHAR(10) | `'auto'` (threshold) or `'manual'` (web) |
| moisture_at_trigger | SMALLINT | Moisture % at time of trigger |
| timestamp | TIMESTAMPTZ | Event time |

---

## 3. Drizzle ORM Schema (TypeScript)

```typescript
// src/db/schema.ts

import { pgTable, serial, varchar, smallint, timestamp, integer } from 'drizzle-orm/pg-core';

export const users = pgTable('users', {
  id: serial('id').primaryKey(),
  username: varchar('username', { length: 50 }).unique().notNull(),
  password: varchar('password', { length: 255 }).notNull(),
  createdAt: timestamp('created_at').defaultNow(),
});

export const rfidUsers = pgTable('rfid_users', {
  id: serial('id').primaryKey(),
  name: varchar('name', { length: 100 }).notNull(),
  rfidUid: varchar('rfid_uid', { length: 20 }).unique().notNull(),
  createdAt: timestamp('created_at').defaultNow(),
});

export const moistureReadings = pgTable('moisture_readings', {
  id: serial('id').primaryKey(),
  value: smallint('value').notNull(),
  rawValue: smallint('raw_value'),
  timestamp: timestamp('timestamp').defaultNow(),
});

export const attendanceLogs = pgTable('attendance_logs', {
  id: serial('id').primaryKey(),
  rfidUid: varchar('rfid_uid', { length: 20 }).notNull(),
  rfidUserId: integer('rfid_user_id').references(() => rfidUsers.id),
  timestamp: timestamp('timestamp').defaultNow(),
});

export const pumpEvents = pgTable('pump_events', {
  id: serial('id').primaryKey(),
  action: varchar('action', { length: 10 }).notNull(),
  triggeredBy: varchar('triggered_by', { length: 10 }).notNull(),
  moistureAtTrigger: smallint('moisture_at_trigger'),
  timestamp: timestamp('timestamp').defaultNow(),
});
```

---

## 4. Common Queries

### Latest moisture reading
```typescript
const latest = await db
  .select()
  .from(moistureReadings)
  .orderBy(desc(moistureReadings.timestamp))
  .limit(1);
```

### Moisture history for last 24h
```typescript
const since = new Date(Date.now() - 24 * 60 * 60 * 1000);
const history = await db
  .select()
  .from(moistureReadings)
  .where(gte(moistureReadings.timestamp, since))
  .orderBy(asc(moistureReadings.timestamp));
```

### Attendance for today with user names
```typescript
const today = new Date();
today.setHours(0, 0, 0, 0);
const logs = await db
  .select({ log: attendanceLogs, user: rfidUsers })
  .from(attendanceLogs)
  .leftJoin(rfidUsers, eq(attendanceLogs.rfidUserId, rfidUsers.id))
  .where(gte(attendanceLogs.timestamp, today))
  .orderBy(desc(attendanceLogs.timestamp));
```

---

## 5. Data Retention Strategy (Future)

For v1 keep all data. For v2 when data grows:
- Keep raw moisture readings for **30 days**
- Keep hourly aggregates for **1 year**
- Keep attendance logs **indefinitely**
- Use a cron job (pg_cron or server-side) to clean old readings