<script lang="ts">
  import Text from "../form/Text.svelte";
  import Range from "../form/Range.svelte";
  import Checkbox from "../form/Checkbox.svelte";
  // export let oscEnabled: boolean;
  export let ip: string;
  export let port: number;
</script>

<!-- <Checkbox label="Enabled" bind:value={oscEnabled} /> -->
<Text label="Address" bind:value={ip} />
<Range label="Port" bind:value={port} min={0} max={Infinity} />

<style>
  /* .global-config {
    background-color: var(--bg-tertiary);
    margin-top: 1em;
  } */
</style>
