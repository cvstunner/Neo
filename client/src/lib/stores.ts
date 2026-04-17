import { writable } from "svelte/store";
import type { AuthUser, AttendanceEntry, RfidUser } from "./api.js";

export const userStore = writable<AuthUser | null>(null);

export const attendanceStore = writable<{
  logs: AttendanceEntry[];
  total: number;
  loading: boolean;
}>({ logs: [], total: 0, loading: false });

export const moistureStore = writable<{
  current: number | null;
  history: { value: number; timestamp: string }[];
  lastUpdated: string | null;
}>({ current: null, history: [], lastUpdated: null });

export const deviceStore = writable<{
  online: boolean;
  lastSeen: string | null;
  ipAddress?: string;
  rssi?: number;
}>({ online: false, lastSeen: null });

export const pumpStore = writable<{
  status: string;
  lastTriggeredAt: string | null;
}>({ status: "off", lastTriggeredAt: null });

export const rfidUsersStore = writable<RfidUser[]>([]);

// ── Toast ─────────────────────────────────────────────────
export const toastStore = writable<
  { id: number; type: "success" | "error" | "info"; message: string }[]
>([]);

let _toastId = 0;
export function showToast(type: "success" | "error" | "info", message: string) {
  const id = ++_toastId;
  toastStore.update((t) => [...t, { id, type, message }]);
  setTimeout(
    () => toastStore.update((t) => t.filter((x) => x.id !== id)),
    4000,
  );
}
