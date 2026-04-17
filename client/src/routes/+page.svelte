<script lang="ts">
  import { onMount } from "svelte";
  import {
    moistureStore,
    deviceStore,
    pumpStore,
    attendanceStore,
  } from "$lib/stores.js";
  import { api } from "$lib/api.js";
  import { format, formatDistanceToNow } from "date-fns";

  let loading = $state(true);
  let chartCanvas = $state<HTMLCanvasElement | null>(null);
  let chartInstance: any = null;

  const mock = { temperature: 24.5, humidity: 62 };

  async function load() {
    try {
      const [m, d, p, a] = await Promise.allSettled([
        api.moisture.latest(),
        api.device.status(),
        api.pump.status(),
        api.attendance.list({ limit: 6 }),
      ]);

      if (m.status === "fulfilled")
        moistureStore.update((s) => ({
          ...s,
          current: m.value.value,
          lastUpdated: m.value.timestamp,
        }));
      if (d.status === "fulfilled")
        deviceStore.set({
          online: d.value.online,
          lastSeen: d.value.lastSeen,
          ipAddress: d.value.ipAddress,
          rssi: d.value.rssi,
        });
      if (p.status === "fulfilled")
        pumpStore.set({
          status: p.value.status,
          lastTriggeredAt: p.value.lastTriggeredAt,
        });
      if (a.status === "fulfilled")
        attendanceStore.set({
          logs: a.value.data,
          total: a.value.total,
          loading: false,
        });

      const hist = await api.moisture
        .history("24h")
        .catch(() => ({ data: [] }));
      if (hist.data.length > 0) renderChart(hist.data);
    } finally {
      loading = false;
    }
  }

  function renderChart(data: { value: number; timestamp: string }[]) {
    if (!chartCanvas) return;
    import("chart.js/auto").then(({ Chart }) => {
      chartInstance?.destroy();
      chartInstance = new Chart(chartCanvas!, {
        type: "line",
        data: {
          labels: data.map((d) => format(new Date(d.timestamp), "HH:mm")),
          datasets: [
            {
              label: "Moisture %",
              data: data.map((d) => d.value),
              borderColor: "#7c6af7",
              backgroundColor: "rgba(124,106,247,0.07)",
              tension: 0.4,
              fill: true,
              pointRadius: 2,
              pointBackgroundColor: "#7c6af7",
            },
          ],
        },
        options: {
          responsive: true,
          maintainAspectRatio: false,
          plugins: { legend: { display: false } },
          scales: {
            x: {
              grid: { color: "#1e1e2e" },
              ticks: { color: "#6b6880", font: { size: 11 } },
            },
            y: {
              min: 0,
              max: 100,
              grid: { color: "#1e1e2e" },
              ticks: {
                color: "#6b6880",
                font: { size: 11 },
                callback: (v: any) => v + "%",
              },
            },
          },
        },
      });
    });
  }

  onMount(() => {
    load();
    return () => chartInstance?.destroy();
  });

  const greeting =
    new Date().getHours() < 12
      ? "morning"
      : new Date().getHours() < 18
        ? "afternoon"
        : "evening";

  let moistureColor = $derived(
    $moistureStore.current == null
      ? "#6b7280"
      : $moistureStore.current < 25
        ? "#f43f5e"
        : $moistureStore.current < 50
          ? "#f59e0b"
          : "#10b981",
  );
  let moistureLabel = $derived(
    $moistureStore.current == null
      ? "No data"
      : $moistureStore.current < 25
        ? "Very Dry"
        : $moistureStore.current < 50
          ? "Dry"
          : $moistureStore.current < 75
            ? "Optimal"
            : "Moist",
  );
</script>

<svelte:head><title>Dashboard — NEO</title></svelte:head>

<div class="page">
  <!-- Header -->
  <div class="page-header">
    <div>
      <h1 class="page-title">Dashboard</h1>
      <p class="page-sub">Good {greeting} — here's NEO's current status.</p>
    </div>
    <div class="device-pill">
      <span
        class="status-dot"
        class:online={$deviceStore.online}
        class:offline={!$deviceStore.online}
      ></span>
      {$deviceStore.online ? "Device Online" : "Device Offline"}
    </div>
  </div>

  {#if loading}
    <div class="stat-grid stagger">
      {#each [0, 1, 2, 3] as _}<div
          class="skeleton"
          style="height:148px"
        ></div>{/each}
    </div>
  {:else}
    <!-- Stats -->
    <div class="stat-grid stagger">
      <!-- Moisture -->
      <div class="stat-card">
        <div class="stat-head">
          <span class="stat-label">Soil Moisture</span>
          <div
            class="stat-icon"
            style="background:{moistureColor}20;color:{moistureColor}"
          >
            <svg
              width="15"
              height="15"
              fill="none"
              stroke="currentColor"
              stroke-width="2"
              viewBox="0 0 24 24"
              ><path d="M12 2.69l5.66 5.66a8 8 0 11-11.31 0z" /></svg
            >
          </div>
        </div>
        <div class="stat-val" style="color:{moistureColor}">
          {$moistureStore.current != null ? $moistureStore.current + "%" : "—"}
        </div>
        <span
          class="badge"
          style="background:{moistureColor}20;color:{moistureColor};border:1px solid {moistureColor}30"
          >{moistureLabel}</span
        >
        {#if $moistureStore.lastUpdated}<p class="stat-meta">
            Updated {formatDistanceToNow(new Date($moistureStore.lastUpdated), {
              addSuffix: true,
            })}
          </p>{/if}
      </div>

      <!-- Pump -->
      <div class="stat-card">
        <div class="stat-head">
          <span class="stat-label">Water Pump</span>
          <div
            class="stat-icon"
            style="background:{$pumpStore.status === 'on'
              ? '#10b98120'
              : '#6b728020'};color:{$pumpStore.status === 'on'
              ? '#10b981'
              : '#6b7280'}"
          >
            <svg
              width="15"
              height="15"
              fill="none"
              stroke="currentColor"
              stroke-width="2"
              viewBox="0 0 24 24"
              ><path
                d="M7 16.3c2.2 0 4-1.83 4-4.05 0-1.16-.57-2.26-1.71-3.19S7.29 6.75 7 5.3c-.29 1.45-1.14 2.84-2.29 3.76S3 11.1 3 12.25c0 2.22 1.8 4.05 4 4.05z"
              /><path
                d="M12.56 6.6A10.97 10.97 0 0014 3.02c.5 2.5 2 4.9 4 6.5s3 3.5 3 5.5a6.98 6.98 0 01-11.91 4.97"
              /></svg
            >
          </div>
        </div>
        <div class="stat-val">
          {$pumpStore.status === "on" ? "Running" : "Idle"}
        </div>
        <span
          class="badge"
          class:badge-green={$pumpStore.status === "on"}
          class:badge-gray={$pumpStore.status !== "on"}
        >
          {$pumpStore.status === "on" ? "Active" : "Standby"}
        </span>
        {#if $pumpStore.lastTriggeredAt}<p class="stat-meta">
            Last run {formatDistanceToNow(
              new Date($pumpStore.lastTriggeredAt),
              { addSuffix: true },
            )}
          </p>{/if}
      </div>

      <!-- Temperature (mock) -->
      <div class="stat-card">
        <div class="stat-head">
          <span class="stat-label">Temperature</span>
          <div
            class="stat-icon"
            style="background:rgba(245,158,11,0.1);color:#f59e0b"
          >
            <svg
              width="15"
              height="15"
              fill="none"
              stroke="currentColor"
              stroke-width="2"
              viewBox="0 0 24 24"
              ><path
                d="M14 14.76V3.5a2.5 2.5 0 00-5 0v11.26a4.5 4.5 0 105 0z"
              /></svg
            >
          </div>
        </div>
        <div class="stat-val">{mock.temperature}°C</div>
        <span class="badge badge-yellow">Mock</span>
        <p class="stat-meta">Sensor not connected yet</p>
      </div>

      <!-- Attendance -->
      <div class="stat-card">
        <div class="stat-head">
          <span class="stat-label">Total Scans</span>
          <div
            class="stat-icon"
            style="background:rgba(124,106,247,0.1);color:#7c6af7"
          >
            <svg
              width="15"
              height="15"
              fill="none"
              stroke="currentColor"
              stroke-width="2"
              viewBox="0 0 24 24"
              ><path d="M17 21v-2a4 4 0 00-4-4H5a4 4 0 00-4 4v2" /><circle
                cx="9"
                cy="7"
                r="4"
              /><path
                d="M23 21v-2a4 4 0 00-3-3.87M16 3.13a4 4 0 010 7.75"
              /></svg
            >
          </div>
        </div>
        <div class="stat-val">{$attendanceStore.total}</div>
        <span class="badge badge-purple">Live</span>
        <p class="stat-meta">All-time RFID scans</p>
      </div>
    </div>

    <!-- Bottom row: chart + recent scans -->
    <div class="bottom-grid">
      <div class="neo-card chart-card">
        <div class="card-header">
          <span class="card-title">Moisture — 24h</span>
          <span class="badge badge-gray">History</span>
        </div>
        {#if $moistureStore.current != null}
          <div class="chart-wrap">
            <canvas bind:this={chartCanvas}></canvas>
          </div>
        {:else}
          <div class="empty-inner">
            <svg
              width="36"
              height="36"
              fill="none"
              stroke="#2a2a3e"
              stroke-width="1.5"
              viewBox="0 0 24 24"
              ><path d="M12 2.69l5.66 5.66a8 8 0 11-11.31 0z" /></svg
            >
            <p>No moisture data yet</p>
            <span>Will appear once NEO device connects</span>
          </div>
        {/if}
      </div>

      <div class="neo-card recent-card">
        <div class="card-header">
          <span class="card-title">Recent Scans</span>
          <a href="/attendance" class="card-link">View all →</a>
        </div>
        {#if $attendanceStore.logs.length}
          <div class="scan-list">
            {#each $attendanceStore.logs.slice(0, 6) as log}
              <div class="scan-row">
                <img
                  src={log.user?.avatarUrl ??
                    `https://ui-avatars.com/api/?name=${encodeURIComponent(log.user?.name ?? "?")}&background=1e1e2e&color=7c6af7&size=30`}
                  alt=""
                  class="scan-avatar"
                />
                <div class="scan-info">
                  <span class="scan-name">{log.user?.name ?? "Unknown"}</span>
                  <span class="scan-uid">{log.rfidUid}</span>
                </div>
                <span class="scan-time"
                  >{format(new Date(log.timestamp), "HH:mm")}</span
                >
              </div>
            {/each}
          </div>
        {:else}
          <div class="empty-inner">
            <p>No scans yet</p>
            <span>Tap an RFID card on the reader</span>
          </div>
        {/if}
      </div>
    </div>
  {/if}
</div>

<style>
  .stat-grid {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(210px, 1fr));
    gap: 16px;
    margin-bottom: 24px;
  }

  .stat-card {
    background: #13131c;
    border: 1px solid #1e1e2e;
    border-radius: 16px;
    padding: 20px;
    display: flex;
    flex-direction: column;
    gap: 8px;
    transition: border-color 0.2s;
  }
  .stat-card:hover {
    border-color: #2a2a3e;
  }
  .stat-head {
    display: flex;
    justify-content: space-between;
    align-items: center;
  }
  .stat-label {
    font-size: 13px;
    color: #7a7690;
    font-weight: 500;
  }
  .stat-icon {
    width: 30px;
    height: 30px;
    border-radius: 8px;
    display: flex;
    align-items: center;
    justify-content: center;
  }
  .stat-val {
    font-family: var(--font-display);
    font-size: 30px;
    font-weight: 700;
    color: #f1f0fb;
    line-height: 1;
    margin: 4px 0;
  }
  .stat-meta {
    font-size: 12px;
    color: #4a4760;
    margin-top: 2px;
  }

  .bottom-grid {
    display: grid;
    grid-template-columns: 1fr 320px;
    gap: 20px;
  }
  @media (max-width: 900px) {
    .bottom-grid {
      grid-template-columns: 1fr;
    }
  }

  .chart-card {
    min-height: 260px;
  }
  .chart-wrap {
    padding: 16px 20px 20px;
    height: 210px;
  }

  .card-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 16px 20px 14px;
    border-bottom: 1px solid #1a1a28;
  }
  .card-title {
    font-size: 14px;
    font-weight: 600;
    color: #c4c0d4;
  }
  .card-link {
    font-size: 13px;
    color: #7c6af7;
    text-decoration: none;
    font-weight: 500;
  }
  .card-link:hover {
    color: #a78bfa;
  }

  .empty-inner {
    display: flex;
    flex-direction: column;
    align-items: center;
    padding: 40px 20px;
    gap: 8px;
    text-align: center;
  }
  .empty-inner p {
    font-size: 14px;
    color: #6b6880;
    font-weight: 500;
  }
  .empty-inner span {
    font-size: 13px;
    color: #3a3a52;
  }

  .scan-list {
    padding: 4px 0;
  }
  .scan-row {
    display: flex;
    align-items: center;
    gap: 11px;
    padding: 9px 18px;
    transition: background 0.15s;
  }
  .scan-row:hover {
    background: #16161f;
  }
  .scan-avatar {
    width: 30px;
    height: 30px;
    border-radius: 50%;
    object-fit: cover;
    flex-shrink: 0;
  }
  .scan-info {
    flex: 1;
    display: flex;
    flex-direction: column;
    gap: 1px;
  }
  .scan-name {
    font-size: 13px;
    font-weight: 500;
    color: #d4d0e8;
  }
  .scan-uid {
    font-family: var(--font-mono);
    font-size: 11px;
    color: #4a4760;
  }
  .scan-time {
    font-size: 12px;
    color: #4a4760;
  }

  .device-pill {
    display: flex;
    align-items: center;
    gap: 8px;
    background: #13131c;
    border: 1px solid #1e1e2e;
    border-radius: 99px;
    padding: 8px 16px;
    font-size: 13px;
    color: #a09cb0;
  }
</style>
