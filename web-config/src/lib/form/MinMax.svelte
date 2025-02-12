<script lang="ts">
  import { uid } from "../../utils";
  export let min: number = 0;
  export let max: number = 1;
  export let mode: "double" | "single" = "double";
  export let minLabel: string = "min";
  export let maxLabel: string = "max";
  export let value: number = 0;
  export let cursorActive: boolean = false;
  export let low: number = 0;
  export let high: number = 100;
  export let step: number = 1;
  let id = uid();
  let color = fillColor();
  const minId = uid();
  const maxId = uid();

  function onMinChange(evt: Event) {
    const target = evt.target as HTMLInputElement;
    const v = +target.value;
    low = Math.min(v, high);
    color = fillColor();
  }

  function onMaxChange(evt: Event) {
    const target = evt.target as HTMLInputElement;
    const v = +target.value;
    high = Math.max(v, low);
    color = fillColor();
  }

  function fillColor() {
    const percent1 = toPercent(low, min, max);
    if (mode === "single") {
      return `linear-gradient(to right, #dadae5 ${percent1} , var(--main) ${percent1} , var(--main) 100%`;
    }
    const percent2 = toPercent(high, min, max);
    return `linear-gradient(to right, #dadae5 ${percent1} , var(--main) ${percent1} , var(--main) ${percent2}, #dadae5 ${percent2})`;
  }

  function toPercent(v: number, a: number, b: number) {
    return `${((v - a) / (b - a)) * 100}%`;
  }
  $: if (mode) {
    color = fillColor();
  }
</script>

<div class="minmax-input" {id}>
  {#if value !== undefined}
    <div class="curr-value">Current reading: {value.toFixed(2)}</div>
  {/if}
  <div class="minmax">
    <span>{min}</span>
    <div class="slider">
      <div class="slider-track" style="--background-color: {color}"></div>
      <input
        type="range"
        {min}
        {max}
        {step}
        bind:value={low}
        on:input={onMinChange}
      />
      {#if mode === "double"}
        <input
          type="range"
          {min}
          {max}
          {step}
          bind:value={high}
          on:input={onMaxChange}
        />
      {/if}
      <span
        class="value {cursorActive ? 'cursorActive' : ''}"
        style="--left:{toPercent(value, min, max)}"
      ></span>
    </div>
    <span>{max}</span>
  </div>

  <div class="inputs">
    <label class="min" for={minId}>{minLabel}:</label>
    <input
      id={minId}
      class="min"
      type="number"
      {min}
      {max}
      {step}
      bind:value={low}
      on:change={onMinChange}
    />
    <label class="max" for={maxId}>{maxLabel}:</label>
    <input
      id={maxId}
      class="max"
      type="number"
      {min}
      {max}
      {step}
      bind:value={high}
      on:change={onMaxChange}
    />
  </div>
</div>

<style>
  .minmax-input {
    grid-auto-flow: column;
    grid-template-rows: auto auto;
    grid-template-columns: auto;
    width: 100%;
  }
  .minmax {
    display: flex;
    gap: 1rem;
    width: 100%;
  }
  .curr-value {
    margin-bottom: 1rem;
  }
  .minmax > span:first-child {
    margin-left: 1rem;
  }
  .minmax > span:last-child {
    margin-right: 1rem;
  }
  .slider {
    display: grid;
    grid-template-rows: 100%;
    grid-template-columns: auto;
    position: relative;
    flex: 1;
  }
  .slider > * {
    grid-area: 1 / 1 / 2 / 2;
  }
  .inputs {
    display: flex;
    align-items: center;
    justify-content: space-evenly;
    width: 100%;
  }
  .inputs > * {
    width: max-content;
  }
  .value {
    position: absolute;
    width: 0;
    height: 0;
    border-left: 0.5em solid transparent;
    border-right: 0.5em solid transparent;
    border-top: 0.5em solid var(--text-color);
    left: var(--left);
  }
  .value.cursorActive {
    border-top-color: var(--main);
  }

  input[type="range"] {
    -webkit-appearance: none;
    -moz-appearance: none;
    appearance: none;
    width: 100%;
    outline: none;
    position: absolute;
    margin: auto;
    padding: 0;
    top: 0;
    bottom: 0;
    background-color: transparent;
    pointer-events: none;
  }
  .slider-track {
    width: 100%;
    height: 5px;
    background: var(--background-color);
    position: absolute;
    margin: auto;
    top: 0;
    bottom: 0;
    border-radius: 5px;
  }
  input[type="range"]::-webkit-slider-runnable-track {
    -webkit-appearance: none;
    height: 5px;
  }
  input[type="range"]::-moz-range-track {
    -webkit-appearance: none;
    height: 5px;
  }
  input[type="range"]::-ms-track {
    -webkit-appearance: none;
    height: 5px;
  }
  input[type="range"]::-webkit-slider-thumb {
    -webkit-appearance: none;
    height: 1.7em;
    width: 1.7em;
    background-color: var(--main);
    cursor: pointer;
    margin-top: -9px;
    pointer-events: auto;
    border-radius: 50%;
  }
  input[type="range"]::-moz-range-thumb {
    -webkit-appearance: none;
    height: 1.7em;
    width: 1.7em;
    background-color: var(--main);
    cursor: pointer;
    margin-top: -9px;
    pointer-events: auto;
    border-radius: 50%;
  }
  input[type="range"]::-ms-thumb {
    -webkit-appearance: none;
    height: 1.7em;
    width: 1.7em;
    background-color: var(--main);
    cursor: pointer;
    margin-top: -9px;
    pointer-events: auto;
    border-radius: 50%;
  }
  input[type="range"]:active::-webkit-slider-thumb {
    background-color: #ffffff;
    border: 1px solid var(--main);
  }
</style>
