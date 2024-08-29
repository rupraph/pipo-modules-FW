<script lang="ts">
  import type { CCConfig, BaseMidiConfig } from "../../types";
  import Range from "../form/Range.svelte";
  import Checkbox from "../form/Checkbox.svelte";
  export let config: CCConfig & BaseMidiConfig;
</script>

<Range
  label="CC number"
  bind:value={config.cc_number}
  min={0}
  max={127}
  step={1}
/>

<Range
  label="Min"
  bind:value={config.min_output}
  min={0}
  max={Math.min(Infinity, config.max_output)}
/>
<Range
  label="Max"
  bind:value={config.max_output}
  min={Math.max(0, config.min_output)}
  max={Infinity}
/>
<Checkbox label="High Resolution" bind:value={config.hires} />

<style>
</style>
