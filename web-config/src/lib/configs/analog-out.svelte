<script lang="ts">
  import type { AnalogOut, AnalogOutKeys } from "../../types";
  import Checkbox from "../form/Checkbox.svelte";
  export let outconfig = {} as AnalogOut;
  console.log(outconfig);
</script>

<div class="analog-out">
  <span class="row-header" style="grid-row:1">Axis</span>
  <span class="row-header" style="grid-row:2">Dir</span>
  <span class="row-header" style="grid-row:3">Out Mode</span>
  {#each Object.entries(outconfig.analogout) as [key, value], i}
    <span class="column" style="grid-column:{i + 2}"> {key} </span>
    <label class="checkbox" style="grid-column:{i + 2};grid-row:{2}">
      <input type="checkbox" bind:checked={value.pindir} />
      <span class="checkmark"></span>
    </label>
    <label class="checkbox" style="grid-column:{i + 2};grid-row:{3}">
      <input
        type="checkbox"
        class="checkbox__input"
        bind:checked={value.outmode}
      />
    </label>
  {/each}
</div>
<div class="out-help">
  <p>Out mode: unticked = PWM, ticked = servo</p>
  <p>OSC address pattern: "/out/A0x"</p>
  <p>OSC range: PWM: 0-1, Servo: 0-180</p>
  <p>OSC port: same as receive</p>
</div>

<style>
  .analog-out {
    display: grid;
    justify-items: center;
    margin-bottom: 1.5em;
    grid-template-rows: 3;
    grid-gap: 1em;
  }

  input[type="checkbox"] {
    width: 0;
    height: 0;
    visibility: hidden;
  }

  .checkbox {
    width: 24px;
    height: 24px;
    border-radius: 50%;
    border: 1px solid #626262;
    background: var(--bg-tertiary) no-repeat center;
  }
  .checkbox:has(input:checked) {
    border-color: var(--main);
    background-color: var(--main);
  }

  .out-help {
    font-size: 12px;
    text-align: left;
    margin-bottom: 1.5em;
  }

  .out-help p {
    line-height: 0.5; /* Adjust the line height as needed */
    margin-bottom: 0.5em; /* Adjust the margin as needed */
  }
</style>
