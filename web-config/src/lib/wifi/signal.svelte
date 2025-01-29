<script lang="ts">
  export let bars = 5;
  export let signal = 4;

  function getHeight(i: number) {
    return i > signal ? 0 : ((i + 1) / bars) * 100;
  }
</script>

<div class="signal">
  {#each Array(bars) as _, i}
    <div
      class="bar"
      style="height: {getHeight(i)}%;
            background: color-mix(in oklab, #f70000 {(1 - signal / bars) *
        100}%, #8fbe00);
    "
    ></div>
  {/each}
</div>

<style scoped>
  .signal {
    width: 100%;
    height: 100%;
    display: flex;
    flex-direction: row;
    gap: 5%;
    transform: scaleY(-1);
  }
  .bar {
    flex: 1;
  }
</style>
