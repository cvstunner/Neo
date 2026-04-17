<script lang="ts">
  import { onMount } from "svelte";
  import { userStore, rfidUsersStore, showToast } from "$lib/stores.js";
  import { api } from "$lib/api.js";
  import type { RfidUser } from "$lib/api.js";

  let loading = $state(true);
  let editing = $state<RfidUser | null>(null);
  let editName = $state("");
  let editEmail = $state("");
  let editUid = $state("");
  let saving = $state(false);

  async function load() {
    loading = true;
    const res = await api.rfidUsers.list();
    rfidUsersStore.set(res.data);
    loading = false;
  }

  function openEdit(u: RfidUser) {
    editing = u;
    editName = u.name;
    editEmail = u.email ?? "";
    editUid = u.rfidUid;
  }

  async function save() {
    if (!editing) return;
    saving = true;
    try {
      await api.rfidUsers.update(editing.id, {
        name: editName || undefined,
        email: editEmail || null,
        rfidUid: editUid || undefined,
      });
      showToast("success", "Saved");
      editing = null;
      load();
    } catch (e: any) {
      showToast("error", e?.message ?? "Update failed");
    } finally {
      saving = false;
    }
  }

  onMount(load);
</script>

<svelte:head><title>Settings — NEO</title></svelte:head>

<div class="page" style="max-width:900px">
  <div class="page-header" style="margin-bottom:32px">
    <div>
      <h1 class="page-title">Settings</h1>
      <p class="page-sub">
        Profile, RFID card assignments, and device preferences
      </p>
    </div>
  </div>

  <!-- Profile -->
  <section class="section">
    <h2 class="sec-title">Profile</h2>
    {#if $userStore}
      <div class="neo-card profile-row">
        <img
          src={$userStore.avatarUrl ??
            `https://ui-avatars.com/api/?name=${encodeURIComponent($userStore.name)}&background=7c6af7&color=fff&size=80`}
          alt="avatar"
          style="width:56px;height:56px;border-radius:50%;object-fit:cover;flex-shrink:0"
        />
        <div style="display:flex;flex-direction:column;gap:2px">
          <span style="font-size:16px;font-weight:600;color:#e2e0f0"
            >{$userStore.name}</span
          >
          <span style="font-size:13px;color:#7a7690">{$userStore.email}</span>
          <span style="font-size:12px;color:#4a4760"
            >Admin · Google Account</span
          >
        </div>
      </div>
    {/if}
  </section>

  <!-- RFID Management -->
  <section class="section">
    <h2 class="sec-title">RFID Card Management</h2>
    <p class="sec-sub">
      Reassign a card to a different person or update their details. This is the
      only place to change an already-registered UID.
    </p>

    {#if loading}
      <div class="skeleton" style="height:120px;border-radius:16px"></div>
    {:else if $rfidUsersStore.length === 0}
      <p style="font-size:13px;color:#6b6880">
        No RFID users registered. Go to RFID Cards to add them.
      </p>
    {:else}
      <div class="neo-card">
        <table class="neo-table">
          <thead>
            <tr
              ><th>Name</th><th>Email</th><th>UID</th><th>Scans</th><th
              ></th></tr
            >
          </thead>
          <tbody>
            {#each $rfidUsersStore as u}
              <tr>
                <td>
                  <div class="person-cell">
                    <img
                      src={u.avatarUrl ??
                        `https://ui-avatars.com/api/?name=${encodeURIComponent(u.name)}&background=1e1e2e&color=7c6af7&size=28`}
                      alt=""
                      class="cell-avatar"
                    />
                    <span class="cell-name">{u.name}</span>
                  </div>
                </td>
                <td class="cell-dim">{u.email ?? "—"}</td>
                <td><span class="uid-chip">{u.rfidUid}</span></td>
                <td class="cell-dim">{u.totalScans ?? 0}</td>
                <td>
                  <button
                    class="btn btn-ghost btn-sm"
                    onclick={() => openEdit(u)}
                  >
                    <svg
                      width="12"
                      height="12"
                      fill="none"
                      stroke="currentColor"
                      stroke-width="2"
                      viewBox="0 0 24 24"
                      ><path
                        d="M11 4H4a2 2 0 00-2 2v14a2 2 0 002 2h14a2 2 0 002-2v-7"
                      /><path
                        d="M18.5 2.5a2.121 2.121 0 013 3L12 15l-4 1 1-4z"
                      /></svg
                    >
                    Edit / Reassign
                  </button>
                </td>
              </tr>
            {/each}
          </tbody>
        </table>
      </div>
    {/if}
  </section>

  <!-- Device config (mock) -->
  <section class="section">
    <h2 class="sec-title">Device Configuration</h2>
    <p class="sec-sub">
      Thresholds and intervals — requires device connection to apply.
    </p>
    <div class="neo-card">
      <div class="cfg-row">
        <div>
          <span class="cfg-label">Moisture Threshold (auto-pump)</span>
          <p class="cfg-hint">
            Pump triggers when moisture falls below this value
          </p>
        </div>
        <div style="display:flex;align-items:center;gap:8px">
          <input
            type="number"
            class="neo-input"
            style="width:72px;text-align:center"
            value="30"
            disabled
          />
          <span style="font-size:13px;color:#7a7690">%</span>
        </div>
      </div>
      <div class="cfg-row" style="border-bottom:none">
        <div>
          <span class="cfg-label">Sensor Read Interval</span>
          <p class="cfg-hint">How often the device reads moisture</p>
        </div>
        <div style="display:flex;align-items:center;gap:8px">
          <input
            type="number"
            class="neo-input"
            style="width:72px;text-align:center"
            value="10"
            disabled
          />
          <span style="font-size:13px;color:#7a7690">min</span>
        </div>
      </div>
      <div style="padding:10px 20px;border-top:1px solid #111118">
        <span class="badge badge-gray">Requires device connection</span>
      </div>
    </div>
  </section>
</div>

<!-- Edit modal -->
{#if editing}
  <div class="modal-overlay" role="dialog" aria-modal="true">
    <div class="modal">
      <div class="modal-header">
        <span class="modal-title">Edit / Reassign Card</span>
        <button class="modal-close" onclick={() => (editing = null)}>✕</button>
      </div>
      <div class="modal-body">
        <div class="field-warn" style="margin-bottom:4px">
          <svg
            width="13"
            height="13"
            fill="none"
            stroke="currentColor"
            stroke-width="2"
            viewBox="0 0 24 24"
            ><path
              d="M10.29 3.86L1.82 18a2 2 0 001.71 3h16.94a2 2 0 001.71-3L13.71 3.86a2 2 0 00-3.42 0z"
            /><line x1="12" y1="9" x2="12" y2="13" /></svg
          >
          Changing the UID reassigns this person to a different physical card.
        </div>
        <div class="field">
          <label class="field-label">Name</label>
          <input class="neo-input" bind:value={editName} />
        </div>
        <div class="field">
          <label class="field-label"
            >Email <span style="font-size:11px;color:#4a4760">optional</span
            ></label
          >
          <input class="neo-input" type="email" bind:value={editEmail} />
        </div>
        <div class="field">
          <label class="field-label">RFID UID</label>
          <input
            class="neo-input"
            style="font-family:var(--font-mono);letter-spacing:1px"
            bind:value={editUid}
          />
          <p class="field-hint">
            Tap new card on device → copy UID from Dashboard
          </p>
        </div>
      </div>
      <div class="modal-footer">
        <button class="btn btn-ghost" onclick={() => (editing = null)}
          >Cancel</button
        >
        <button class="btn btn-primary" onclick={save} disabled={saving}>
          {saving ? "Saving…" : "Save Changes"}
        </button>
      </div>
    </div>
  </div>
{/if}

<style>
  .section {
    margin-bottom: 36px;
  }
  .sec-title {
    font-family: var(--font-display);
    font-size: 16px;
    font-weight: 700;
    color: #d4d0e8;
    margin-bottom: 4px;
  }
  .sec-sub {
    font-size: 13px;
    color: #6b6880;
    margin-bottom: 14px;
  }

  .profile-row {
    display: flex;
    align-items: center;
    gap: 18px;
    padding: 20px 24px;
  }

  .cfg-row {
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 16px 20px;
    border-bottom: 1px solid #111118;
    gap: 16px;
    flex-wrap: wrap;
  }
  .cfg-label {
    font-size: 14px;
    font-weight: 500;
    color: #c4c0d4;
  }
  .cfg-hint {
    font-size: 12px;
    color: #6b6880;
    margin-top: 2px;
  }
</style>
