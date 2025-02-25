<script lang="ts">
  export let enabled: boolean;
</script>

{#if !enabled}
  <p class="warning">Note: Disabled in quick config</p>
{/if}

<style>
  .warning {
    color: var(--red);
    margin-bottom: 1em;
  }
</style>
