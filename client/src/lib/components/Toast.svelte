<script lang="ts">
  import { toastStore } from "$lib/stores.js";
</script>

<div class="toast-wrap">
  {#each $toastStore as t (t.id)}
    <div class="toast toast-{t.type}" role="alert">
      <span class="t-icon"
        >{t.type === "success" ? "✓" : t.type === "error" ? "✕" : "●"}</span
      >
      <span>{t.message}</span>
    </div>
  {/each}
</div>
