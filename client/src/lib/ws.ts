import {
  attendanceStore,
  moistureStore,
  deviceStore,
  pumpStore,
  showToast,
} from "./stores.js";
import type { AttendanceEntry } from "./api.js";

let socket: WebSocket | null = null;
let reconnectTimer: ReturnType<typeof setTimeout>;
let dead = false;

export function connectWS() {
  if (dead) return;
  const url = import.meta.env.VITE_WS_URL ?? "ws://localhost:3000/ws";
  try {
    socket = new WebSocket(url);
  } catch {
    schedule();
    return;
  }

  socket.onopen = () => console.log("[WS] connected");

  socket.onmessage = ({ data }) => {
    try {
      const msg = JSON.parse(data);

      if (msg.type === "moisture.update") {
        moistureStore.update((s) => ({
          ...s,
          current: msg.value,
          lastUpdated: msg.timestamp,
        }));
      }

      if (msg.type === "rfid.scan") {
        const entry: AttendanceEntry = {
          id: Date.now(),
          rfidUid: msg.uid,
          timestamp: msg.timestamp,
          user: msg.user ?? null,
        };
        attendanceStore.update((s) => ({
          ...s,
          logs: [entry, ...s.logs],
          total: s.total + 1,
        }));
        showToast("info", `Scan: ${msg.user?.name ?? `Unknown (${msg.uid})`}`);
      }

      if (msg.type === "device.status") {
        deviceStore.set({
          online: msg.online,
          lastSeen: msg.timestamp,
          ipAddress: msg.ip,
          rssi: msg.rssi,
        });
      }

      if (msg.type === "pump.status") {
        pumpStore.update((s) => ({
          ...s,
          status: msg.status,
          lastTriggeredAt: msg.timestamp,
        }));
      }
    } catch {
      /* ignore parse errors */
    }
  };

  socket.onclose = schedule;
  socket.onerror = () => socket?.close();
}

export function disconnectWS() {
  dead = true;
  clearTimeout(reconnectTimer);
  socket?.close();
}

function schedule() {
  clearTimeout(reconnectTimer);
  reconnectTimer = setTimeout(connectWS, 4000);
}
