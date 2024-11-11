<script lang="ts">
  import { onMount, onDestroy } from "svelte";
  import { sound } from "./sound";
  import { pipoio } from "../../pipoio";
  let error: string;
  let angle = 0;
  let value = 0;
  export let ccNumber = 0;
  export let max = 127;
  export let min = 0;
  function interp(t: number) {
    return Math.round((t - min) * (max - min) + min);
  }

  onMount(async () => {
    pipoio.on("controlChange", ({ control, value }) => {
      if (control !== ccNumber) return;
      angle = ((value - min) / (max - min)) * 270;
      value = interp(value);
    });
  });

  onDestroy(() => {
    sound.stop();
  });
</script>

<div class="layout">
  {#if error}
    <div class="error">
      <p>{error}</p>
    </div>
  {:else}
    <div class="pote-layout">
      <div class="pote" style="transform: rotate({angle + 135}deg)">
        <span></span>
      </div>
      <div class="deg bl">{min}</div>
      <div class="deg tl">{interp(0.33)}</div>
      <div class="deg tr">{interp(0.66)}</div>
      <div class="deg br">{max}</div>
      <div class="value">{value}</div>
    </div>
  {/if}
</div>

<style>
  .pote-layout {
    display: grid;
    grid-template-rows: 1em 100px 1em;
    grid-template-columns: 1em 100px 1em;
  }

  .pote {
    grid-area: 2 / 2 / 3 / 3;
    border-radius: 50%;
    box-shadow: 0 0 0 10px silver;
    overflow: hidden;
    display: grid;
    grid-template-rows: auto 5px auto;
    grid-template-columns: auto 33%;
  }
  .pote span {
    background-color: white;
    grid-area: 2 / 2 / 3 / 3;
    border-radius: 2px;
    margin: 0 5px;
  }
  .deg.tl {
    grid-area: 1 / 1 / 2 / 2;
    text-align: left;
    transform: rotate(-45deg);
  }
  .deg.tr {
    grid-area: 1 / 3 / 2 / 4;
    text-align: right;
    transform: rotate(45deg);
  }
  .deg.bl {
    grid-area: 3 / 1 / 4 / 2;
    text-align: left;
    transform: rotate(45deg);
  }
  .deg.br {
    grid-area: 3 / 3 / 4 / 4;
    text-align: right;
    transform: rotate(-45deg);
  }
  .value {
    grid-area: 2 / 2 / 3 / 3;
    text-align: center;
    font-size: 2em;
    margin: auto;
  }
</style>
