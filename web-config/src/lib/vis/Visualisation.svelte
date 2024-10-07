<script lang="ts" generics="T extends PipoTypes">
  import { onMount, onDestroy } from "svelte";
  import Collapse from "../collapse.svelte";
  import { sound } from "./sound";
  import type { Axis, MidiConfig, PipoConfig } from "../../types";
  import Piano from "./Piano.svelte";
  import Potentiometer from "./Potentiometer.svelte";
  import Cube from "./Cube.svelte";
  import { pipoType as type } from "../../services";

  export let config: PipoConfig<T>;
  function getMidiConfigs() {
    return Object.entries(config.engine["engine-midi"]) as unknown as [
      Axis<typeof $type>,
      MidiConfig,
    ][];
  }

  /*
    {#each getMidiConfigs() as [axis, midiconfig]}
    <h3 id={axis}>{axis}</h3>
    <section>
      {#if midiconfig.th_mode}
        <!-- <Piano /> -->
      {:else}
        <!-- <Potentiometer ccNumber={midiconfig.cc_number} /> -->
      {/if}
    </section>
  {/each} 
  <Cube {config} />
*/
</script>

{#if $type === "motion"}
  <Cube {config} />
  <div class="motion-pots">
    <span>
      Yaw
      <Potentiometer
        ccNumber={config.engine["engine-midi"].yaw.cc_number}
        min={config.engine["engine-midi"].yaw.min}
        max={config.engine["engine-midi"].yaw.max}
      />
    </span>
    <span>
      Pitch
      <Potentiometer
        ccNumber={config.engine["engine-midi"].pitch.cc_number}
        min={config.engine["engine-midi"].pitch.min}
        max={config.engine["engine-midi"].pitch.max}
      />
    </span>
    <span>
      Roll
      <Potentiometer
        ccNumber={config.engine["engine-midi"].roll.cc_number}
        min={config.engine["engine-midi"].roll.min}
        max={config.engine["engine-midi"].roll.max}
      />
    </span>
    <span>
      X acceleration
      <Potentiometer
        ccNumber={config.engine["engine-midi"].accX.cc_number}
        min={config.engine["engine-midi"].accX.min}
        max={config.engine["engine-midi"].accX.max}
      />
    </span>
    <span>
      Y acceleration
      <Potentiometer
        ccNumber={config.engine["engine-midi"].accY.cc_number}
        min={config.engine["engine-midi"].accY.min}
        max={config.engine["engine-midi"].accY.max}
      />
    </span>
    <span>
      Z acceleration
      <Potentiometer
        ccNumber={config.engine["engine-midi"].accZ.cc_number}
        min={config.engine["engine-midi"].accZ.min}
        max={config.engine["engine-midi"].accZ.max}
      />
    </span>
  </div>
{/if}

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
