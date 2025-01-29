<script lang="ts">
  import { minMax } from "../../constants";
  import { pipoType as type } from "../../services";
  import type { Axis, BaseMidiConfig } from "../../types";
  import Range from "../form/Range.svelte";
  import Collapse from "../collapse.svelte";
  export let config: BaseMidiConfig;
  export let axis: Axis<typeof $type>;
</script>

<Collapse title="Input">
  {#if minMax[$type][axis]}
    <Range
      label="Min"
      min={minMax[$type][axis].min}
      max={Math.min(minMax[$type][axis].max, config.max_input)}
      bind:value={config.min_input}
    />
    <Range
      label="Max"
      min={Math.max(minMax[$type][axis].min, config.min_input)}
      max={minMax[$type][axis].max}
      bind:value={config.max_input}
    />
  {/if}
</Collapse>

<style>
</style>
