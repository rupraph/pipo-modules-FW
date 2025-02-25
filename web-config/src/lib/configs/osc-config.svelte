<script lang="ts">
  import type { OscConfig } from "../../types";
  import Text from "../form/Text.svelte";
  import Range from "../form/Range.svelte";
  import Checkbox from "../form/Checkbox.svelte";
  import DisabledWarning from "./disabled-warning.svelte";
  export let osc: OscConfig;
</script>

<Text label="Address" bind:value={osc.osc_addr} maxlength={255} />
<Checkbox label="Raw Mode" bind:value={osc.mode_raw} />
{#if !osc.mode_raw}
  <Range label="Min" bind:value={osc.osc_min} min={0} max={osc.osc_max} />
  <Range
    label="Max"
    bind:value={osc.osc_max}
    min={osc.osc_min}
    max={Infinity}
  />
{/if}

<DisabledWarning bind:enabled={osc.enabled} />
