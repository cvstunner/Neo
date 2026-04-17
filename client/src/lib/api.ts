const BASE = import.meta.env.VITE_API_URL ?? "http://localhost:3000";

async function req<T>(path: string, init?: RequestInit): Promise<T> {
  const res = await fetch(`${BASE}${path}`, {
    credentials: "include",
    headers: { "Content-Type": "application/json", ...(init?.headers ?? {}) },
    ...init,
  });
  if (!res.ok) {
    const err = await res.json().catch(() => ({ error: "Request failed" }));
    throw err;
  }
  return res.json();
}

export const api = {
  auth: {
    me: () => req<AuthUser>("/api/auth/me"),
    logout: () => req("/api/auth/logout", { method: "POST" }),
    googleUrl: () => `${BASE}/api/auth/google`,
  },

  rfidUsers: {
    list: () => req<{ data: RfidUser[] }>("/api/rfid-users"),
    check: (p: { uid?: string; name?: string }) => {
      const q = new URLSearchParams(
        Object.fromEntries(Object.entries(p).filter(([, v]) => v)),
      ).toString();
      return req<{
        uidTaken?: boolean;
        nameTaken?: boolean;
        existingUser?: RfidUser;
      }>(`/api/rfid-users/check?${q}`);
    },
    create: (b: { name: string; email?: string; rfidUid: string }) =>
      req<RfidUser>("/api/rfid-users", {
        method: "POST",
        body: JSON.stringify(b),
      }),
    update: (
      id: number,
      b: { name?: string; email?: string | null; rfidUid?: string },
    ) =>
      req<RfidUser>(`/api/rfid-users/${id}`, {
        method: "PATCH",
        body: JSON.stringify(b),
      }),
    delete: (id: number) => req(`/api/rfid-users/${id}`, { method: "DELETE" }),
  },

  attendance: {
    list: (p?: AttendanceQuery) => {
      const q = p
        ? new URLSearchParams(
            Object.fromEntries(
              Object.entries(p)
                .filter(([, v]) => v != null)
                .map(([k, v]) => [k, String(v)]),
            ),
          ).toString()
        : "";
      return req<{ data: AttendanceEntry[]; total: number }>(
        `/api/attendance${q ? "?" + q : ""}`,
      );
    },
    summary: (p?: { startDate?: string; endDate?: string }) => {
      const q = p
        ? new URLSearchParams(p as Record<string, string>).toString()
        : "";
      return req<{ data: SummaryRow[] }>(
        `/api/attendance/summary${q ? "?" + q : ""}`,
      );
    },
    import: (rows: ImportRow[]) =>
      req<{ imported: number; errors: string[] }>("/api/attendance/import", {
        method: "POST",
        body: JSON.stringify({ rows }),
      }),
    template: () =>
      req<{ columns: string[]; example: ImportRow[]; notes: string[] }>(
        "/api/attendance/import/template",
      ),
    delete: (id: number) => req(`/api/attendance/${id}`, { method: "DELETE" }),
  },

  moisture: {
    latest: () =>
      req<{ value: number | null; timestamp: string | null }>(
        "/api/moisture/latest",
      ),
    history: (range = "24h") =>
      req<{ data: { value: number; timestamp: string }[] }>(
        `/api/moisture/history?range=${range}`,
      ),
  },

  pump: {
    status: () =>
      req<{
        status: string;
        lastTriggeredAt: string | null;
        lastTriggeredBy: string | null;
      }>("/api/pump/status"),
    control: (action: "on" | "off") =>
      req("/api/pump/control", {
        method: "POST",
        body: JSON.stringify({ action }),
      }),
  },

  device: {
    status: () =>
      req<{
        online: boolean;
        lastSeen: string | null;
        ipAddress?: string;
        rssi?: number;
      }>("/api/device/status"),
  },
};

// ── Types ─────────────────────────────────────────────────
export interface AuthUser {
  id: number;
  email: string;
  name: string;
  avatarUrl?: string;
}
export interface RfidUser {
  id: number;
  name: string;
  email?: string | null;
  rfidUid: string;
  avatarUrl?: string | null;
  createdAt: string;
  totalScans?: number;
}
export interface AttendanceEntry {
  id: number;
  rfidUid: string;
  timestamp: string;
  user: {
    id: number;
    name: string;
    email?: string | null;
    avatarUrl?: string | null;
  } | null;
}
export interface SummaryRow {
  userName: string | null;
  date: string;
  count: number;
}
export interface ImportRow {
  name: string;
  rfid_uid: string;
  timestamp: string;
}
export interface AttendanceQuery {
  date?: string;
  userId?: string;
  limit?: number;
  offset?: number;
  startDate?: string;
  endDate?: string;
}
