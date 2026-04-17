import * as XLSX from "xlsx";
import { format } from "date-fns";
import type { AttendanceEntry, ImportRow } from "./api.js";

export function exportToExcel(
  logs: AttendanceEntry[],
  filename = "attendance",
) {
  const rows = logs.map((l) => ({
    Name: l.user?.name ?? "Unknown",
    RFID_UID: l.rfidUid,
    Date: format(new Date(l.timestamp), "yyyy-MM-dd"),
    Time: format(new Date(l.timestamp), "HH:mm:ss"),
    Timestamp: l.timestamp,
    Email: l.user?.email ?? "",
  }));
  const ws = XLSX.utils.json_to_sheet(rows);
  ws["!cols"] = [
    { wch: 20 },
    { wch: 14 },
    { wch: 12 },
    { wch: 10 },
    { wch: 26 },
    { wch: 28 },
  ];
  const wb = XLSX.utils.book_new();
  XLSX.utils.book_append_sheet(wb, ws, "Attendance");
  XLSX.writeFile(wb, `${filename}_${format(new Date(), "yyyy-MM-dd")}.xlsx`);
}

export function exportToPDF(
  logs: AttendanceEntry[],
  title = "NEO Attendance Report",
) {
  const html = `<!DOCTYPE html><html><head><meta charset="utf-8"><title>${title}</title>
  <style>*{margin:0;padding:0;box-sizing:border-box}body{font-family:Helvetica Neue,sans-serif;color:#111;padding:40px}
  h1{font-size:22px;font-weight:700;margin-bottom:4px}.sub{color:#666;font-size:13px;margin-bottom:28px}
  table{width:100%;border-collapse:collapse;font-size:13px}
  th{background:#f5f5f5;padding:10px 14px;text-align:left;font-weight:600;border-bottom:2px solid #e5e5e5}
  td{padding:9px 14px;border-bottom:1px solid #f0f0f0}tr:nth-child(even)td{background:#fafafa}
  .uid{font-family:monospace;color:#666;font-size:11px}.footer{margin-top:20px;font-size:12px;color:#999}
  </style></head><body>
  <h1>${title}</h1>
  <p class="sub">Generated: ${format(new Date(), "MMMM d, yyyy HH:mm")} · ${logs.length} records</p>
  <table><thead><tr><th>#</th><th>Name</th><th>RFID UID</th><th>Date</th><th>Time</th></tr></thead>
  <tbody>${logs
    .map(
      (l, i) => `<tr><td>${i + 1}</td><td>${l.user?.name ?? "Unknown"}</td>
    <td class="uid">${l.rfidUid}</td>
    <td>${format(new Date(l.timestamp), "MMM d, yyyy")}</td>
    <td>${format(new Date(l.timestamp), "HH:mm:ss")}</td></tr>`,
    )
    .join("")}
  </tbody></table>
  <p class="footer">NEO · ${new Date().toISOString()}</p>
  </body></html>`;
  const win = window.open("", "_blank");
  if (win) {
    win.document.write(html);
    win.document.close();
    win.print();
  }
}

export function parseImportFile(file: File): Promise<ImportRow[]> {
  return new Promise((resolve, reject) => {
    const reader = new FileReader();
    reader.onload = (e) => {
      try {
        const data = new Uint8Array(e.target!.result as ArrayBuffer);
        const wb = XLSX.read(data, { type: "array" });
        const ws = wb.Sheets[wb.SheetNames[0]];
        const rows = XLSX.utils.sheet_to_json<Record<string, string>>(ws);
        resolve(
          rows.map((r) => ({
            name: String(r["name"] ?? r["Name"] ?? ""),
            rfid_uid: String(r["rfid_uid"] ?? r["RFID_UID"] ?? r["uid"] ?? ""),
            timestamp: String(r["timestamp"] ?? r["Timestamp"] ?? ""),
          })),
        );
      } catch (e) {
        reject(e);
      }
    };
    reader.onerror = reject;
    reader.readAsArrayBuffer(file);
  });
}

export function downloadImportTemplate() {
  const rows = [
    { name: "Rushil", rfid_uid: "A1B2C3D4", timestamp: "2026-04-10T09:00:00Z" },
    { name: "Mom", rfid_uid: "F4E3D2C1", timestamp: "2026-04-10T08:30:00Z" },
  ];
  const ws = XLSX.utils.json_to_sheet(rows);
  ws["!cols"] = [{ wch: 20 }, { wch: 16 }, { wch: 28 }];
  const wb = XLSX.utils.book_new();
  XLSX.utils.book_append_sheet(wb, ws, "Import Template");
  XLSX.writeFile(wb, "neo_attendance_import_template.xlsx");
}
