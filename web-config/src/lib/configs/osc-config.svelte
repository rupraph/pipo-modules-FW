<script lang="ts">
  import type { OscConfig } from "../../types";
  import Text from "../form/Text.svelte";
  import Range from "../form/Range.svelte";
  import Checkbox from "../form/Checkbox.svelte";
  import DisabledWarning from "./disabled-warning.svelte";
  import Input from "../form/Input.svelte";

  export let osc: OscConfig;
  export let oscEnabled: boolean;
  export let ip: string;
  export let port: number;
</script>

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

<DisabledWarning enabled={!osc.enabled} feature="OSC" />
