<script lang="ts">
  export let bars = 5;
  export let signal = 4;
  export let disconnected = false;
  function getHeight(disconnected: boolean, i: number, signal: number) {
    return i > signal && !disconnected ? 0 : ((i + 1) / bars) * 100;
  }
  function getBorder(disconnected: boolean) {
    return disconnected ? "1px solid var(--bg-lighter)" : "none";
  }
  function getColor(disconnected: boolean, signal: number, bars: number) {
    return disconnected
      ? "transparent"
      : signal / bars < 0.33
        ? "var(--red)"
        : signal / bars < 0.66
          ? "var(--main-darker)"
          : "var(--green)";
  }
</script>

<div class="signal">
  {#each Array(bars) as _, i}
    <div
      class="bar"
      style="height: {getHeight(disconnected, i, signal)}%;
            background: {getColor(disconnected, signal, bars)};
            border:{getBorder(disconnected)}; 
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
