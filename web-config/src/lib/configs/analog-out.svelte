<script lang="ts">
  import type { AnalogOut, AnalogOutKeys } from "../../types";
  import Checkbox from "../form/Checkbox.svelte";
  import Tooltip from "../tooltip/Tooltip.svelte";
  export let outconfig;
</script>

<div class="analog-out">
  <div class="row">
    <div class="header">Pin</div>
    <div class="header">Direction</div>
    <div class="header">Out Mode</div>
  </div>
  {#each Object.entries(outconfig.analogout) as [key, value]}
    <div class="column">
      <div class="cell">{key}</div>
      <div class="cell">
        <label class="checkbox">
          <input type="checkbox" bind:checked={value.pindir} />
          <span class="checkmark"></span>
        </label>
      </div>

      <div class="cell">
        <label class="checkbox">
          <input type="checkbox" bind:checked={value.outmode} />
          <span class="checkmark"></span>
        </label>
      </div>
    </div>
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
    display: flex;
    flex-direction: row;
    justify-items: center;
    margin-bottom: 1.5em;
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

  .row {
    display: flex;
    flex-direction: column;
    margin-right: 1em;
  }

  .column {
    display: flex;
    flex-direction: column;
    margin-right: 1em;
  }

  .header {
    /* font-weight: bold; */
    border-bottom: 2px solid #ccc;
    padding-bottom: 0.5em;
    margin-bottom: 0.5em;
  }

  .cell {
    padding: 0.5em;
    /* border-bottom: 1px solid #ccc; */
  }

  .cell:last-child {
    border-bottom: none;
  }

  .checkbox {
    display: flex;
    align-items: center;
    position: relative;
    cursor: pointer;
    user-select: none;
  }

  .checkbox input {
    position: absolute;
    opacity: 0;
    cursor: pointer;
    height: 0;
    width: 0;
  }

  .checkmark {
    width: 24px;
    height: 24px;
    border-radius: 50%;
    border: 1px solid #626262;
    background: var(--bg-tertiary);
  }

  .checkbox input:checked + .checkmark {
    border-color: var(--main);
    background-color: var(--main);
  }
</style>
