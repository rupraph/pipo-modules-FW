<script lang="ts">
  import Input from "./Input.svelte";
  export let label: string;
  export let min: number = 0;
  export let max: number = 1;
  export let low: number = 0;
  export let high: number = 100;
  export let step: number = 1;
  let id = Math.random().toString(36).substring(2);
  let color = fillColor();
  function onMinChange(v: number) {
    low = Math.min(v, high);
    color = fillColor();
  }
  function onMaxChange(v: number) {
    high = Math.max(v, low);
    color = fillColor();
  }
  function fillColor() {
    const percent1 = (low / max) * 100;
    const percent2 = (high / max) * 100;
    return `linear-gradient(to right, #dadae5 ${percent1}% , var(--main) ${percent1}% , var(--main) ${percent2}%, #dadae5 ${percent2}%)`;
  }
</script>

<Input {label} {id}>
  <div class="container">
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
  </div>
</Input>

<style>
  .container {
    position: relative;
    width: 100%;
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
