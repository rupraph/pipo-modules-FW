<script lang="ts" generics="T extends PipoTypes">
  import { minMax } from "../../constants";
  import { pipoType as type } from "../../services";
  import type { BaseMidiConfig, PipoKeys, PipoTypes } from "../../types";
  import Range from "../form/Range.svelte";
  import Collapse from "../collapse.svelte";
  export let config: BaseMidiConfig;
  export let axis: PipoKeys[T];
  function getMinMax() {
    return minMax[$type as T][axis];
  }
</script>

<Collapse title="Input">
  {@const { min, max } = getMinMax()}
  <Range
    label="Min"
    {min}
    max={Math.min(max, config.max_input)}
    bind:value={config.min_input}
  />
  <Range
    label="Max"
    min={Math.max(min, config.min_input)}
    {max}
    bind:value={config.max_input}
  />
</Collapse>

<style>
</style>
