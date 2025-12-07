<script lang="ts">
  import { pipoio } from "../pipoio";
  import { onMount, onDestroy } from "svelte";

  let battPercentage: number | null = null;
  let isPlugged: boolean = false;
  let intervalId: number;

  async function fetchBatteryLevel() {
    try {
      const { data } = await pipoio.get("/battlevel", { timeout: 2000 });
      const value = parseInt(data);

      // -1 means plugged
      if (value === -1) {
        isPlugged = true;
        battPercentage = null;
      } else {
        isPlugged = false;
        battPercentage = value;
      }
    } catch (error) {
      console.error("Failed to fetch battery level:", error);
    }
  }

  onMount(() => {
    fetchBatteryLevel();
    intervalId = setInterval(fetchBatteryLevel, 5000);
  });

  onDestroy(() => {
    clearInterval(intervalId);
  });

  // Calculate battery level for visual representation
  $: batteryLevel = isPlugged ? 100 : (battPercentage ?? 0);
  $: batteryColor =
    batteryLevel > 50 ? "var(--main)" : batteryLevel > 20 ? "orange" : "red";
</script>

<nav class="battery-status">
  <div class="battery-info">
    <div class="battery-icon">
      <div class="battery-body">
        <div
          class="battery-level"
          style="width: {batteryLevel}%; background-color: {batteryColor};"
        />
      </div>
      <div class="battery-tip" />
    </div>
  </div>
  <span class="battery-text">
    {#if battPercentage === null && !isPlugged}
      <span>...</span>
    {:else if isPlugged}
      <span>Plugged</span>
    {:else}
      <span>{battPercentage}%</span>
    {/if}
  </span>
</nav>

<style>
  nav.battery-status {
    width: 100%;
    display: flex;
    align-items: center;
    box-sizing: border-box;
    justify-content: flex-end;
  }

  .battery-info {
    display: flex;
    align-items: center;
    gap: 8px;
  }

  .battery-icon {
    display: flex;
    align-items: center;
    gap: 1px;
  }

  .battery-body {
    position: relative;
    width: 24px;
    height: 10px;
    border: 2px solid var(--text-color);
    border-radius: 3px;
    background-color: var(--bg-primary);
    overflow: hidden;
  }

  .battery-level {
    position: absolute;
    left: 0;
    top: 0;
    height: 100%;
    transition:
      width 0.3s ease,
      background-color 0.3s ease;
  }

  .battery-tip {
    width: 3px;
    height: 8px;
    background-color: var(--text-color);
    border-radius: 0 2px 2px 0;
  }

  .battery-text {
    font-size: 14px;
    font-weight: 500;
    color: var(--text-color);
    min-width: 50px;
  }
</style>
