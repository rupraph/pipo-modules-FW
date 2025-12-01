<script lang="ts">
  import Text from "../form/Text.svelte";
  import Range from "../form/Range.svelte";
  import Switch from "../form/Switch.svelte";
  export let ip: string;
  export let port: number;
  export let battEnabled: boolean;
</script>

<Text label="Address" bind:value={ip} />
<Range label="Port" bind:value={port} min={0} max={Infinity} />
<Switch label="Send Battery Level" bind:value={battEnabled} design="slider" />
