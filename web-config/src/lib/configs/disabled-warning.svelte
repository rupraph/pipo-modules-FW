<script lang="ts">
  export let enabled: boolean;
  export let feature: string;

  $: console.log({ enabled, feature });
</script>

{#if enabled}
  <p class="warning">
    Warning: {feature} is disabled from quick config for this axis.
  </p>
{/if}

<style>
  .warning {
    color: var(--red);
    margin-bottom: 1em;
  }
</style>
