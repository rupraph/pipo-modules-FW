<script lang="ts">
  import { Signal, SignalHigh, SignalLow, SignalZero } from "lucide-svelte";

  export let bars = 5;
  export let signal = 4;
  export let disconnected = false;

  function getColor(disconnected: boolean, signal: number, bars: number) {
    return disconnected
      ? "var(--bg-lighter)"
      : signal / bars < 0.33
        ? "var(--red)"
        : signal / bars < 0.66
          ? "var(--main-lighter)"
          : "var(--main)";
  }

  function getIcon(signal: number, bars: number, disconnected: boolean) {
    if (disconnected) return SignalZero;
    const ratio = signal / bars;
    if (ratio < 0.25) return SignalZero;
    if (ratio < 0.5) return SignalLow;
    if (ratio < 0.75) return SignalHigh;
    return Signal;
  }

  $: Icon = getIcon(signal, bars, disconnected);
  $: color = getColor(disconnected, signal, bars);
</script>

<div class="signal">
  <svelte:component this={Icon} size={24} style="color: {color}" />
</div>

<style scoped>
  .signal {
    display: flex;
    align-items: center;
    justify-content: center;
  }
</style>
