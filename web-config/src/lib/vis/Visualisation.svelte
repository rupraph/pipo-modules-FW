<script lang="ts" generics="T extends PipoTypes">
  import Config from "../configs/config.svelte";

  import type { ConfigByAxis, PipoTypes } from "../../types";
  import { schema } from "../../schema";
  import { pipoType as type } from "../../services";
  import Potentiometer from "./Potentiometer.svelte";
  import Cube from "./Cube.svelte";

  export let configByAxis: ConfigByAxis<T>;
  function getConfig() {
    return (Object.keys(configByAxis) as Array<keyof ConfigByAxis<T>>).map(
      (axis) => {
        return {
          label: schema[$type as T][axis].label,
          ...configByAxis[axis].midi,
        };
      }
    );
  }
</script>

<!-- <Cube {config} /> -->
<div class="motion-pots">
  {#each getConfig() as { cc_nb, cc_min, cc_max, label }, i}
    <span>
      {label}
      <Potentiometer ccNumber={cc_nb} min={cc_min} max={cc_max} />
    </span>
  {/each}
</div>

<style>
  .motion-pots {
    display: grid;
    grid-template-rows: auto auto;
    grid-template-columns: auto auto auto;
    row-gap: 1em;
    justify-content: space-evenly;
  }
  .motion-pots span {
    display: flex;
    flex-direction: column;
    gap: 0.5em;
  }
</style>
