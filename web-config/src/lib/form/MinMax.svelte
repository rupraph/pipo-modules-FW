<script lang="ts">
  import { uid } from "../../utils";
  import Input from "./Input.svelte";
  export let label: string;
  export let min: number = 0;
  export let max: number = 1;
  export let minLabel: string = "min";
  export let maxLabel: string = "max";
  export let value: number | undefined = undefined;
  export let low: number = 0;
  export let high: number = 100;
  export let step: number = 1;
  let id = uid();
  let color = fillColor();
  const minId = uid();
  const maxId = uid();
  function onMinChange(v: number) {
    low = Math.min(v, high);
    color = fillColor();
  }
  function onMaxChange(v: number) {
    high = Math.max(v, low);
    color = fillColor();
  }
  function fillColor() {
    const percent1 = toPercent(low, min, max);
    const percent2 = toPercent(high, min, max);
    return `linear-gradient(to right, #dadae5 ${percent1} , var(--main) ${percent1} , var(--main) ${percent2}, #dadae5 ${percent2})`;
  }
  function toPercent(v: number, a: number, b: number) {
    return `${((v - a) / (b - a)) * 100}%`;
  }
</script>

<Input class="minmax-input" {label} {id}>
  <div class="minmax">
    <span>{min}</span>
    <div class="slider">
      <div class="slider-track" style="--background-color: {color}"></div>
      <input
        type="range"
        {min}
        {max}
        bind:value={low}
        on:input={(v) => onMinChange(v.target.value)}
      />
      <input
        type="range"
        {min}
        {max}
        bind:value={high}
        on:input={(v) => onMaxChange(v.target.value)}
      />
      <span class="value" style="--left:{toPercent(value, min, max)}"></span>
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
      bind:value={low}
      on:change={(v) => onMinChange(v.target.value)}
    />
    <label class="max" for={maxId}>{maxLabel}:</label>
    <input
      id={maxId}
      class="max"
      type="number"
      {min}
      {max}
      bind:value={high}
      on:change={(v) => onMaxChange(v.target.value)}
    />
  </div>
</Input>

<style>
  .minmax {
    display: flex;
    gap: 1rem;
    grid-area: 1 / 1 / 2 / 3;
    width: 100%;
  }
  .minmax > span:first-child {
    margin-left: 1rem;
  }
  .minmax > span:last-child {
    margin-right: 1rem;
  }
  .slider {
    position: relative;
    flex: 1;
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
    border-top: 0.5em solid var(--main);
    left: var(--left);
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
