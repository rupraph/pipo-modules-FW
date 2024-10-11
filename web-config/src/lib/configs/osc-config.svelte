<script lang="ts">
  import type { OscConfig } from "../../types";
  import Text from "../form/Text.svelte";
  import Range from "../form/Range.svelte";
  import Checkbox from "../form/Checkbox.svelte";
  import DisabledWarning from "./disabled-warning.svelte";

  export let osc: OscConfig;
  export let oscEnabled: boolean;
  export let ip: string;
  export let port: number;
</script>

<DisabledWarning enabled={!osc.enabled} feature="OSC" />
<h4>OSC Network settings</h4>
<Checkbox label="OSC Enabled" bind:value={oscEnabled} />
<Text label="OSC IP" bind:value={ip} />
<Range label="OSC Port" bind:value={port} />
<!-- <Checkbox label="Enabled" bind:value={oscconf.enabled} /> -->
<Checkbox label="Mode_raw" bind:value={osc.mode_raw} />
{#if !osc.mode_raw}
  <Range label="OSC Min" bind:value={osc.osc_min} />
  <Range label="OSC Max" bind:value={osc.osc_max} />
{/if}
