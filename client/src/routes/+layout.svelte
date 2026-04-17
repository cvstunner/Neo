<script lang="ts">
  import "../app.css";
  import { onMount } from "svelte";
  import { page } from "$app/stores";
  import { goto } from "$app/navigation";
  import { userStore } from "$lib/stores.js";
  import { api } from "$lib/api.js";
  // import { connectWS, disconnectWS } from '$lib/ws.js';
  import { connectWS } from "$lib/ws";
  import Sidebar from "$lib/components/Sidebar.svelte";
  import Toast from "$lib/components/Toast.svelte";

  let { children } = $props();

  $: isPublic = $page.url.pathname.startsWith("/login");

  onMount(async () => {
    if (!isPublic) {
      try {
        const me = await api.auth.me();
        userStore.set(me);
        connectWS();
      } catch {
        goto("/login");
      }
    }
    return () => disconnectWS();
  });
</script>

{#if isPublic}
  {@render children()}
{:else}
  <div class="shell">
    <Sidebar />
    <main class="main">
      {@render children()}
    </main>
  </div>
{/if}

<Toast />

<style>
  .shell {
    display: flex;
    min-height: 100vh;
  }
  .main {
    flex: 1;
    margin-left: 220px;
    min-height: 100vh;
    background: var(--color-bg-base);
    overflow-x: hidden;
  }
</style>
