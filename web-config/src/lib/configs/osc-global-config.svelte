<script lang="ts">
  import Text from "../form/Text.svelte";
  import Range from "../form/Range.svelte";
  export let ip: string;
  export let port: number;
</script>

<Text label="Address" bind:value={ip} />
<Range label="Port" bind:value={port} min={0} max={Infinity} />
