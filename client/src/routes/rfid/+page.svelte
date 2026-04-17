<script lang="ts">
  import { onMount } from "svelte";
  import { rfidUsersStore, showToast } from "$lib/stores.js";
  import { api } from "$lib/api.js";
  import type { RfidUser } from "$lib/api.js";

  let loading = $state(true);
  let showModal = $state(false);

  let name = $state("");
  let email = $state("");
  let rfidUid = $state("");
  let checking = $state(false);
  let saving = $state(false);
  let uidTaken = $state(false);
  let nameTaken = $state(false);
  let existingUser = $state<RfidUser | null>(null);

  let debounce: ReturnType<typeof setTimeout>;

  async function load() {
    loading = true;
    const res = await api.rfidUsers.list();
    rfidUsersStore.set(res.data);
    loading = false;
  }

  function onInput() {
    clearTimeout(debounce);
    debounce = setTimeout(check, 500);
  }

  async function check() {
    if (!rfidUid && !name) {
      uidTaken = false;
      nameTaken = false;
      existingUser = null;
      return;
    }
    checking = true;
    try {
      const res = await api.rfidUsers.check({
        uid: rfidUid || undefined,
        name: name || undefined,
      });
      uidTaken = res.uidTaken ?? false;
      nameTaken = res.nameTaken ?? false;
      existingUser = res.existingUser ?? null;
    } finally {
      checking = false;
    }
  }

  async function register() {
    if (uidTaken || nameTaken) return;
    saving = true;
    try {
      await api.rfidUsers.create({ name, email: email || undefined, rfidUid });
      showToast("success", `${name} registered`);
      name = "";
      email = "";
      rfidUid = "";
      uidTaken = false;
      nameTaken = false;
      existingUser = null;
      showModal = false;
      load();
    } catch (e: any) {
      showToast("error", e?.message ?? "Registration failed");
    } finally {
      saving = false;
    }
  }

  async function remove(u: RfidUser) {
    if (!confirm(`Remove ${u.name}?`)) return;
    await api.rfidUsers.delete(u.id);
    showToast("success", `${u.name} removed`);
    load();
  }

  function openModal() {
    name = "";
    email = "";
    rfidUid = "";
    uidTaken = false;
    nameTaken = false;
    existingUser = null;
    showModal = true;
  }

  onMount(load);
</script>

<svelte:head><title>RFID Cards — NEO</title></svelte:head>

<div class="page">
  <div class="page-header">
    <div>
      <h1 class="page-title">RFID Cards</h1>
      <p class="page-sub">
        Register and manage cards/tags for attendance tracking
      </p>
    </div>
    <button class="btn btn-primary" onclick={openModal}>
      <svg
        width="14"
        height="14"
        fill="none"
        stroke="currentColor"
        stroke-width="2.5"
        viewBox="0 0 24 24"
        ><line x1="12" y1="5" x2="12" y2="19" /><line
          x1="5"
          y1="12"
          x2="19"
          y2="12"
        /></svg
      >
      Register Card
    </button>
  </div>

  <div class="info-banner">
    <svg
      width="15"
      height="15"
      fill="none"
      stroke="currentColor"
      stroke-width="2"
      viewBox="0 0 24 24"
      ><rect x="2" y="5" width="20" height="14" rx="2" /><path
        d="M2 10h20"
      /></svg
    >
    <span
      ><strong>How to find a UID:</strong> Tap your card on the NEO device — the
      UID appears live on the Dashboard. Copy it from there, then register it here.</span
    >
  </div>

  {#if loading}
    <div class="card-grid stagger">
      {#each [0, 1, 2] as _}<div
          class="skeleton"
          style="height:148px"
        ></div>{/each}
    </div>
  {:else if $rfidUsersStore.length === 0}
    <div class="empty-state">
      <svg
        width="52"
        height="52"
        fill="none"
        stroke="#2a2a3e"
        stroke-width="1.2"
        viewBox="0 0 24 24"
        ><rect x="2" y="5" width="20" height="14" rx="2" /><path
          d="M2 10h20"
        /></svg
      >
      <p>No cards registered yet</p>
      <span>Click "Register Card" to link an RFID tag to a person</span>
    </div>
  {:else}
    <div class="card-grid stagger">
      {#each $rfidUsersStore as u}
        <div class="user-card">
          <div class="uc-top">
            <img
              src={u.avatarUrl ??
                `https://ui-avatars.com/api/?name=${encodeURIComponent(u.name)}&background=1e1e2e&color=7c6af7&size=48`}
              alt={u.name}
              class="uc-avatar"
            />
            <div class="uc-info">
              <span class="uc-name">{u.name}</span>
              {#if u.email}<span class="uc-email">{u.email}</span>{/if}
            </div>
          </div>
          <div class="uc-uid">
            <span
              style="font-size:11px;color:#4a4760;font-weight:600;text-transform:uppercase;letter-spacing:0.5px"
              >UID</span
            >
            <span class="uid-chip">{u.rfidUid}</span>
          </div>
          <div class="uc-footer">
            <span class="scan-count">
              <svg
                width="12"
                height="12"
                fill="none"
                stroke="#10b981"
                stroke-width="2"
                viewBox="0 0 24 24"><path d="M9 11l3 3L22 4" /></svg
              >
              {u.totalScans ?? 0} scans
            </span>
            <button class="btn btn-danger btn-sm" onclick={() => remove(u)}
              >Remove</button
            >
          </div>
        </div>
      {/each}
    </div>
  {/if}
</div>

<!-- Register modal -->
{#if showModal}
  <div class="modal-overlay" role="dialog" aria-modal="true">
    <div class="modal">
      <div class="modal-header">
        <span class="modal-title">Register RFID Card</span>
        <button class="modal-close" onclick={() => (showModal = false)}
          >✕</button
        >
      </div>
      <div class="modal-body">
        <div class="field">
          <label class="field-label">Full Name *</label>
          <input
            class="neo-input"
            placeholder="e.g. Rushil"
            bind:value={name}
            oninput={onInput}
          />
          {#if nameTaken && existingUser}
            <div class="field-warn">
              <svg
                width="13"
                height="13"
                fill="none"
                stroke="currentColor"
                stroke-width="2"
                viewBox="0 0 24 24"
                ><circle cx="12" cy="12" r="10" /><line
                  x1="12"
                  y1="8"
                  x2="12"
                  y2="12"
                /></svg
              >
              "{name}" already has card <strong>{existingUser.rfidUid}</strong>.
              Go to Settings to reassign.
            </div>
          {/if}
        </div>

        <div class="field">
          <label class="field-label"
            >Email <span style="font-size:11px;color:#4a4760;font-weight:400"
              >optional</span
            ></label
          >
          <input
            class="neo-input"
            type="email"
            placeholder="name@email.com"
            bind:value={email}
          />
        </div>

        <div class="field">
          <label class="field-label">RFID UID *</label>
          <input
            class="neo-input"
            style="font-family:var(--font-mono);letter-spacing:1px"
            placeholder="e.g. A1B2C3D4"
            bind:value={rfidUid}
            oninput={onInput}
          />
          {#if checking}
            <p class="field-hint">Checking…</p>
          {:else if uidTaken && existingUser}
            <div class="field-warn">
              <svg
                width="13"
                height="13"
                fill="none"
                stroke="currentColor"
                stroke-width="2"
                viewBox="0 0 24 24"
                ><circle cx="12" cy="12" r="10" /><line
                  x1="12"
                  y1="8"
                  x2="12"
                  y2="12"
                /></svg
              >
              Already registered to <strong>{existingUser.name}</strong>.
              Reassign from Settings.
            </div>
          {:else if rfidUid.length >= 4}
            <p class="field-ok">✓ UID available</p>
          {/if}
          <p class="field-hint">
            Tap card on NEO → UID appears on Dashboard live feed
          </p>
        </div>
      </div>
      <div class="modal-footer">
        <button class="btn btn-ghost" onclick={() => (showModal = false)}
          >Cancel</button
        >
        <button
          class="btn btn-primary"
          disabled={!name || !rfidUid || uidTaken || nameTaken || saving}
          onclick={register}
        >
          {saving ? "Registering…" : "Register Card"}
        </button>
      </div>
    </div>
  </div>
{/if}

<style>
  .info-banner {
    display: flex;
    align-items: flex-start;
    gap: 10px;
    background: rgba(124, 106, 247, 0.07);
    border: 1px solid rgba(124, 106, 247, 0.18);
    border-radius: 12px;
    padding: 13px 16px;
    font-size: 13px;
    color: #a09cb0;
    margin-bottom: 24px;
  }
  .info-banner svg {
    color: #7c6af7;
    flex-shrink: 0;
    margin-top: 1px;
  }
  .info-banner strong {
    color: #d4d0e8;
  }

  .card-grid {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(250px, 1fr));
    gap: 16px;
  }

  .empty-state {
    display: flex;
    flex-direction: column;
    align-items: center;
    padding: 64px 24px;
    gap: 10px;
    text-align: center;
  }
  .empty-state p {
    font-size: 15px;
    font-weight: 500;
    color: #6b6880;
  }
  .empty-state span {
    font-size: 13px;
    color: #3a3a52;
  }

  .user-card {
    background: #13131c;
    border: 1px solid #1e1e2e;
    border-radius: 16px;
    padding: 18px;
    display: flex;
    flex-direction: column;
    gap: 14px;
    transition: border-color 0.2s;
  }
  .user-card:hover {
    border-color: #2a2a3e;
  }

  .uc-top {
    display: flex;
    gap: 12px;
    align-items: center;
  }
  .uc-avatar {
    width: 44px;
    height: 44px;
    border-radius: 50%;
    object-fit: cover;
    flex-shrink: 0;
  }
  .uc-info {
    display: flex;
    flex-direction: column;
  }
  .uc-name {
    font-size: 15px;
    font-weight: 600;
    color: #e2e0f0;
  }
  .uc-email {
    font-size: 12px;
    color: #6b6880;
  }

  .uc-uid {
    display: flex;
    align-items: center;
    gap: 8px;
    background: #0e0e16;
    border: 1px solid #1a1a28;
    border-radius: 8px;
    padding: 7px 12px;
  }

  .uc-footer {
    display: flex;
    justify-content: space-between;
    align-items: center;
  }
  .scan-count {
    display: flex;
    align-items: center;
    gap: 5px;
    font-size: 12px;
    color: #6b6880;
  }
</style>
