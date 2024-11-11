<script lang="ts">
  import type { CCConfig, BaseMidiConfig } from "../../types";
  import Range from "../form/Range.svelte";
  import Checkbox from "../form/Checkbox.svelte";
  export let config: CCConfig & BaseMidiConfig;
</script>

<Range label="CC Number" bind:value={config.cc_nb} min={0} max={127} step={1} />

<Range
  label="CC Out Min"
  bind:value={config.cc_min}
  min={0}
  max={Math.min(Infinity, config.cc_max)}
/>
<Range
  label="CC Out Max"
  bind:value={config.cc_max}
  min={Math.max(0, config.cc_min)}
  max={Infinity}
/>
<Checkbox label="High Res mode" bind:value={config.hires} />

<style>
</style>
