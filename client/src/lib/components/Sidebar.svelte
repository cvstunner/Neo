<script lang="ts">
  import { page } from "$app/stores";
  import { goto } from "$app/navigation";
  import { userStore } from "$lib/stores.js";
  import { api } from "$lib/api.js";

  const nav = [
    {
      href: "/dashboard",
      label: "Dashboard",
      icon: `<svg width="17" height="17" fill="none" stroke="currentColor" stroke-width="2" viewBox="0 0 24 24"><rect x="3" y="3" width="7" height="7" rx="1"/><rect x="14" y="3" width="7" height="7" rx="1"/><rect x="3" y="14" width="7" height="7" rx="1"/><rect x="14" y="14" width="7" height="7" rx="1"/></svg>`,
    },
    {
      href: "/attendance",
      label: "Attendance",
      icon: `<svg width="17" height="17" fill="none" stroke="currentColor" stroke-width="2" viewBox="0 0 24 24"><path d="M9 11l3 3L22 4"/><path d="M21 12v7a2 2 0 01-2 2H5a2 2 0 01-2-2V5a2 2 0 012-2h11"/></svg>`,
    },
    {
      href: "/rfid",
      label: "RFID Cards",
      icon: `<svg width="17" height="17" fill="none" stroke="currentColor" stroke-width="2" viewBox="0 0 24 24"><rect x="2" y="5" width="20" height="14" rx="2"/><path d="M2 10h20"/></svg>`,
    },
    {
      href: "/settings",
      label: "Settings",
      icon: `<svg width="17" height="17" fill="none" stroke="currentColor" stroke-width="2" viewBox="0 0 24 24"><circle cx="12" cy="12" r="3"/><path d="M12 1v2M12 21v2M4.22 4.22l1.42 1.42M18.36 18.36l1.42 1.42M1 12h2M21 12h2M4.22 19.78l1.42-1.42M18.36 5.64l1.42-1.42"/></svg>`,
    },
  ];

  async function logout() {
    await api.auth.logout().catch(() => {});
    userStore.set(null);
    goto("/login");
  }
</script>

<aside class="sidebar">
  <!-- Logo -->
  <div class="logo">
    <div class="logo-mark">N</div>
    <span class="logo-text">NEO</span>
  </div>

  <!-- Nav -->
  <nav class="nav-list">
    {#each nav as item}
      {@const active = $page.url.pathname.startsWith(item.href)}
      <a href={item.href} class="nav-item" class:active>
        <span class="nav-icon">{@html item.icon}</span>
        <span>{item.label}</span>
        {#if active}<span class="active-pip"></span>{/if}
      </a>
    {/each}
  </nav>

  <!-- Profile -->
  <div class="sidebar-bottom">
    {#if $userStore}
      <div class="profile-row">
        <img
          src={$userStore.avatarUrl ??
            `https://ui-avatars.com/api/?name=${encodeURIComponent($userStore.name)}&background=7c6af7&color=fff&size=32`}
          alt="avatar"
          class="p-avatar"
        />
        <div class="p-info">
          <span class="p-name">{$userStore.name.split(" ")[0]}</span>
          <span class="p-email">{$userStore.email}</span>
        </div>
        <button onclick={logout} class="logout-btn" title="Sign out">
          <svg
            width="15"
            height="15"
            fill="none"
            stroke="currentColor"
            stroke-width="2"
            viewBox="0 0 24 24"
          >
            <path
              d="M9 21H5a2 2 0 01-2-2V5a2 2 0 012-2h4M16 17l5-5-5-5M21 12H9"
            />
          </svg>
        </button>
      </div>
    {/if}
  </div>
</aside>

<style>
  .sidebar {
    width: 220px;
    min-height: 100vh;
    background: #0d0d15;
    border-right: 1px solid #1a1a28;
    display: flex;
    flex-direction: column;
    position: fixed;
    left: 0;
    top: 0;
    bottom: 0;
    z-index: 100;
  }

  .logo {
    display: flex;
    align-items: center;
    gap: 10px;
    padding: 24px 20px 20px;
    border-bottom: 1px solid #181824;
  }
  .logo-mark {
    width: 32px;
    height: 32px;
    border-radius: 9px;
    background: linear-gradient(135deg, #7c6af7, #a78bfa);
    display: flex;
    align-items: center;
    justify-content: center;
    font-family: var(--font-display);
    font-weight: 800;
    font-size: 16px;
    color: #fff;
    box-shadow: 0 4px 14px rgba(124, 106, 247, 0.4);
    flex-shrink: 0;
  }
  .logo-text {
    font-family: var(--font-display);
    font-weight: 700;
    font-size: 17px;
    letter-spacing: 3px;
    color: #f1f0fb;
  }

  .nav-list {
    flex: 1;
    padding: 12px;
    display: flex;
    flex-direction: column;
    gap: 2px;
  }

  .nav-item {
    display: flex;
    align-items: center;
    gap: 10px;
    padding: 10px 12px;
    border-radius: 10px;
    color: #6b6880;
    text-decoration: none;
    font-size: 14px;
    font-weight: 500;
    transition: all 0.18s;
    position: relative;
    overflow: hidden;
  }
  .nav-item:hover {
    background: #16161f;
    color: #c4c0d4;
  }
  .nav-item.active {
    background: rgba(124, 106, 247, 0.1);
    color: #a78bfa;
  }
  .nav-icon {
    display: flex;
    align-items: center;
    flex-shrink: 0;
  }

  .active-pip {
    position: absolute;
    right: 0;
    top: 22%;
    bottom: 22%;
    width: 3px;
    background: #7c6af7;
    border-radius: 99px 0 0 99px;
  }

  .sidebar-bottom {
    padding: 12px;
    border-top: 1px solid #181824;
  }

  .profile-row {
    display: flex;
    align-items: center;
    gap: 9px;
    padding: 10px;
    border-radius: 10px;
    background: #13131c;
    border: 1px solid #1e1e2e;
  }
  .p-avatar {
    width: 30px;
    height: 30px;
    border-radius: 50%;
    object-fit: cover;
    flex-shrink: 0;
  }
  .p-info {
    flex: 1;
    min-width: 0;
    display: flex;
    flex-direction: column;
  }
  .p-name {
    font-size: 13px;
    font-weight: 600;
    color: #e2e0f0;
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
  }
  .p-email {
    font-size: 11px;
    color: #6b6880;
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
  }

  .logout-btn {
    background: none;
    border: none;
    color: #6b6880;
    cursor: pointer;
    padding: 4px;
    border-radius: 6px;
    display: flex;
    flex-shrink: 0;
    transition:
      color 0.2s,
      background 0.2s;
  }
  .logout-btn:hover {
    color: #f43f5e;
    background: rgba(244, 63, 94, 0.1);
  }
</style>
