<script lang="ts">
  import { onMount } from "svelte";
  import { rfidUsersStore, showToast } from "$lib/stores.js";
  import { api } from "$lib/api.js";
  import {
    exportToExcel,
    exportToPDF,
    parseImportFile,
    downloadImportTemplate,
  } from "$lib/exportImport.js";
  import { format } from "date-fns";
  import type { AttendanceEntry, ImportRow } from "$lib/api.js";

  let loading = $state(true);
  let logs = $state<AttendanceEntry[]>([]);
  let total = $state(0);
  let currentPage = $state(0);
  let filterDate = $state(format(new Date(), "yyyy-MM-dd"));
  let filterUser = $state("");
  const LIMIT = 25;

  // import modal
  let showImport = $state(false);
  let importFile = $state<File | null>(null);
  let importPreview = $state<ImportRow[]>([]);
  let importLoading = $state(false);

  async function load() {
    loading = true;
    try {
      const res = await api.attendance.list({
        date: filterDate || undefined,
        userId: filterUser || undefined,
        limit: LIMIT,
        offset: currentPage * LIMIT,
      });
      logs = res.data;
      total = res.total;
    } finally {
      loading = false;
    }
  }

  async function loadUsers() {
    const res = await api.rfidUsers.list();
    rfidUsersStore.set(res.data);
  }

  async function del(id: number) {
    if (!confirm("Delete this record?")) return;
    await api.attendance.delete(id);
    showToast("success", "Deleted");
    load();
  }

  async function handleFile(e: Event) {
    const f = (e.target as HTMLInputElement).files?.[0];
    if (!f) return;
    importFile = f;
    try {
      importPreview = await parseImportFile(f);
    } catch {
      showToast("error", "Could not parse file");
    }
  }

  async function submitImport() {
    importLoading = true;
    try {
      const res = await api.attendance.import(importPreview);
      showToast("success", `Imported ${res.imported} records`);
      if (res.errors.length)
        showToast("error", `${res.errors.length} rows skipped`);
      showImport = false;
      importPreview = [];
      importFile = null;
      load();
    } catch {
      showToast("error", "Import failed");
    } finally {
      importLoading = false;
    }
  }

  function prevPage() {
    currentPage--;
    load();
  }
  function nextPage() {
    currentPage++;
    load();
  }

  onMount(() => {
    load();
    loadUsers();
  });

  $effect(() => {
    if (filterDate !== undefined || filterUser !== undefined) currentPage = 0;
  });
</script>

<svelte:head><title>Attendance — NEO</title></svelte:head>

<div class="page">
  <div class="page-header">
    <div>
      <h1 class="page-title">Attendance</h1>
      <p class="page-sub">
        {total} records{filterDate
          ? ` · ${format(new Date(filterDate + "T00:00"), "MMM d, yyyy")}`
          : ""}
      </p>
    </div>
    <div class="header-actions">
      <button class="btn btn-ghost" onclick={() => (showImport = true)}>
        <svg
          width="14"
          height="14"
          fill="none"
          stroke="currentColor"
          stroke-width="2"
          viewBox="0 0 24 24"
          ><path
            d="M21 15v4a2 2 0 01-2 2H5a2 2 0 01-2-2v-4M7 10l5 5 5-5M12 15V3"
          /></svg
        >
        Import
      </button>
      <button class="btn btn-ghost" onclick={() => exportToExcel(logs)}>
        <svg
          width="14"
          height="14"
          fill="none"
          stroke="currentColor"
          stroke-width="2"
          viewBox="0 0 24 24"
          ><path
            d="M21 15v4a2 2 0 01-2 2H5a2 2 0 01-2-2v-4M17 8l-5-5-5 5M12 3v12"
          /></svg
        >
        Excel
      </button>
      <button class="btn btn-ghost" onclick={() => exportToPDF(logs)}>
        <svg
          width="14"
          height="14"
          fill="none"
          stroke="currentColor"
          stroke-width="2"
          viewBox="0 0 24 24"
          ><path
            d="M14 2H6a2 2 0 00-2 2v16a2 2 0 002 2h12a2 2 0 002-2V8z"
          /><polyline points="14 2 14 8 20 8" /></svg
        >
        PDF
      </button>
    </div>
  </div>

  <!-- Filters -->
  <div class="filters">
    <div class="filter-grp">
      <label class="filter-lbl">Date</label>
      <input
        type="date"
        class="neo-input"
        style="width:180px"
        bind:value={filterDate}
        onchange={load}
      />
    </div>
    <div class="filter-grp">
      <label class="filter-lbl">Person</label>
      <select
        class="neo-input"
        style="width:180px"
        bind:value={filterUser}
        onchange={load}
      >
        <option value="">All people</option>
        {#each $rfidUsersStore as u}
          <option value={String(u.id)}>{u.name}</option>
        {/each}
      </select>
    </div>
    <button
      class="btn btn-ghost btn-sm"
      onclick={() => {
        filterDate = "";
        filterUser = "";
        load();
      }}>Clear</button
    >
  </div>

  <!-- Table -->
  <div class="neo-card">
    {#if loading}
      <div style="padding:16px;display:flex;flex-direction:column;gap:8px">
        {#each [0, 1, 2, 3, 4] as _}
          <div class="skeleton" style="height:44px;border-radius:8px"></div>
        {/each}
      </div>
    {:else if logs.length === 0}
      <div class="empty-state">
        <svg
          width="44"
          height="44"
          fill="none"
          stroke="#2a2a3e"
          stroke-width="1.5"
          viewBox="0 0 24 24"
          ><path d="M9 11l3 3L22 4" /><path
            d="M21 12v7a2 2 0 01-2 2H5a2 2 0 01-2-2V5a2 2 0 012-2h11"
          /></svg
        >
        <p>No attendance records found</p>
        <span>Scan an RFID card on NEO to create records</span>
      </div>
    {:else}
      <table class="neo-table">
        <thead>
          <tr>
            <th>#</th><th>Person</th><th>RFID UID</th>
            <th>Date</th><th>Time</th><th>Status</th><th></th>
          </tr>
        </thead>
        <tbody>
          {#each logs as log, i}
            <tr>
              <td
                style="color:#3a3a52;font-family:var(--font-mono);font-size:12px"
                >{currentPage * LIMIT + i + 1}</td
              >
              <td>
                <div class="person-cell">
                  <img
                    src={log.user?.avatarUrl ??
                      `https://ui-avatars.com/api/?name=${encodeURIComponent(log.user?.name ?? "?")}&background=1e1e2e&color=7c6af7&size=28`}
                    alt=""
                    class="cell-avatar"
                  />
                  <div>
                    <span class="cell-name">{log.user?.name ?? "—"}</span>
                    {#if log.user?.email}<span class="cell-email"
                        >{log.user.email}</span
                      >{/if}
                  </div>
                </div>
              </td>
              <td><span class="uid-chip">{log.rfidUid}</span></td>
              <td class="cell-dim"
                >{format(new Date(log.timestamp), "MMM d, yyyy")}</td
              >
              <td class="cell-dim"
                >{format(new Date(log.timestamp), "HH:mm:ss")}</td
              >
              <td>
                {#if log.user}
                  <span class="badge badge-green">Identified</span>
                {:else}
                  <span class="badge badge-yellow">Unknown</span>
                {/if}
              </td>
              <td>
                <button
                  class="del-btn"
                  onclick={() => del(log.id)}
                  title="Delete"
                >
                  <svg
                    width="13"
                    height="13"
                    fill="none"
                    stroke="currentColor"
                    stroke-width="2"
                    viewBox="0 0 24 24"
                    ><polyline points="3 6 5 6 21 6" /><path
                      d="M19 6l-1 14H6L5 6M10 11v6M14 11v6M9 6V4h6v2"
                    /></svg
                  >
                </button>
              </td>
            </tr>
          {/each}
        </tbody>
      </table>

      {#if total > LIMIT}
        <div class="pagination">
          <button
            class="btn btn-ghost btn-sm"
            disabled={currentPage === 0}
            onclick={prevPage}>← Prev</button
          >
          <span class="page-info"
            >{currentPage + 1} / {Math.ceil(total / LIMIT)}</span
          >
          <button
            class="btn btn-ghost btn-sm"
            disabled={(currentPage + 1) * LIMIT >= total}
            onclick={nextPage}>Next →</button
          >
        </div>
      {/if}
    {/if}
  </div>
</div>

<!-- Import modal -->
{#if showImport}
  <div class="modal-overlay" role="dialog" aria-modal="true">
    <div class="modal" style="max-width:540px">
      <div class="modal-header">
        <span class="modal-title">Import Attendance</span>
        <button
          class="modal-close"
          onclick={() => {
            showImport = false;
            importPreview = [];
            importFile = null;
          }}>✕</button
        >
      </div>
      <div class="modal-body">
        <div class="info-tip">
          <svg
            width="14"
            height="14"
            fill="none"
            stroke="currentColor"
            stroke-width="2"
            viewBox="0 0 24 24"
            ><circle cx="12" cy="12" r="10" /><line
              x1="12"
              y1="8"
              x2="12"
              y2="12"
            /><line x1="12" y1="16" x2="12.01" y2="16" /></svg
          >
          Your file must match the template format.
          <button class="link-btn" onclick={downloadImportTemplate}
            >Download template →</button
          >
        </div>

        <label class="file-drop" class:has-file={!!importFile}>
          <input
            type="file"
            accept=".xlsx,.xls,.csv"
            onchange={handleFile}
            style="display:none"
          />
          <svg
            width="26"
            height="26"
            fill="none"
            stroke="currentColor"
            stroke-width="1.5"
            viewBox="0 0 24 24"
            ><path
              d="M21 15v4a2 2 0 01-2 2H5a2 2 0 01-2-2v-4M7 10l5 5 5-5M12 15V3"
            /></svg
          >
          <span
            >{importFile
              ? importFile.name
              : "Click to select .xlsx or .csv"}</span
          >
        </label>

        {#if importPreview.length > 0}
          <div>
            <p style="font-size:13px;color:#7a7690;margin-bottom:10px">
              {importPreview.length} rows detected
            </p>
            <div class="preview-wrap">
              <table class="neo-table" style="font-size:12px">
                <thead
                  ><tr><th>Name</th><th>RFID UID</th><th>Timestamp</th></tr
                  ></thead
                >
                <tbody>
                  {#each importPreview.slice(0, 5) as r}
                    <tr
                      ><td>{r.name}</td><td
                        ><span class="uid-chip">{r.rfid_uid}</span></td
                      ><td style="color:#7a7690">{r.timestamp}</td></tr
                    >
                  {/each}
                </tbody>
              </table>
              {#if importPreview.length > 5}
                <p style="font-size:12px;color:#4a4760;padding:8px 12px">
                  +{importPreview.length - 5} more rows…
                </p>
              {/if}
            </div>
          </div>
        {/if}
      </div>
      <div class="modal-footer">
        <button
          class="btn btn-ghost"
          onclick={() => {
            showImport = false;
            importPreview = [];
            importFile = null;
          }}>Cancel</button
        >
        <button
          class="btn btn-primary"
          disabled={importPreview.length === 0 || importLoading}
          onclick={submitImport}
        >
          {importLoading ? "Importing…" : `Import ${importPreview.length} rows`}
        </button>
      </div>
    </div>
  </div>
{/if}

<style>
  .filters {
    display: flex;
    gap: 12px;
    align-items: flex-end;
    margin-bottom: 20px;
    flex-wrap: wrap;
  }
  .filter-grp {
    display: flex;
    flex-direction: column;
    gap: 6px;
  }
  .filter-lbl {
    font-size: 12px;
    color: #7a7690;
    font-weight: 500;
  }

  .empty-state {
    display: flex;
    flex-direction: column;
    align-items: center;
    padding: 60px 24px;
    gap: 10px;
    text-align: center;
  }
  .empty-state p {
    font-size: 14px;
    font-weight: 500;
    color: #6b6880;
  }
  .empty-state span {
    font-size: 13px;
    color: #3a3a52;
  }

  .del-btn {
    background: none;
    border: none;
    color: #3a3a52;
    cursor: pointer;
    padding: 6px;
    border-radius: 6px;
    transition:
      color 0.2s,
      background 0.2s;
  }
  .del-btn:hover {
    color: #f43f5e;
    background: rgba(244, 63, 94, 0.1);
  }

  .info-tip {
    display: flex;
    align-items: center;
    gap: 8px;
    font-size: 13px;
    color: #7a7690;
    background: #16161f;
    border: 1px solid #1e1e2e;
    border-radius: 10px;
    padding: 11px 14px;
    flex-wrap: wrap;
  }
  .link-btn {
    background: none;
    border: none;
    color: #7c6af7;
    cursor: pointer;
    font-size: 13px;
    font-family: var(--font-sans);
    padding: 0;
    text-decoration: underline;
  }

  .file-drop {
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    gap: 10px;
    border: 2px dashed #1e1e2e;
    border-radius: 12px;
    padding: 28px;
    cursor: pointer;
    color: #6b6880;
    font-size: 14px;
    transition: all 0.2s;
    text-align: center;
  }
  .file-drop:hover,
  .file-drop.has-file {
    border-color: #7c6af7;
    background: rgba(124, 106, 247, 0.04);
    color: #a78bfa;
  }

  .preview-wrap {
    border: 1px solid #1a1a28;
    border-radius: 10px;
    overflow-x: auto;
  }
</style>
