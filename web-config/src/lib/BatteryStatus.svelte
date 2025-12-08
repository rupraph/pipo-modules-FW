<script lang="ts">
  import { pipoio } from "../pipoio";
  import { onMount, onDestroy } from "svelte";
  import {
    BatteryCharging,
    BatteryFull,
    BatteryMedium,
    BatteryLow,
  } from "lucide-svelte";

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

  // Calculate battery level and icon
  $: batteryLevel = battPercentage ?? 0;
  $: batteryColor =
    batteryLevel > 50 ? "var(--main)" : batteryLevel > 35 ? "orange" : "red";
</script>

<nav class="battery-status">
  <div class="battery-info">
    <div class="battery-icon">
      {#if isPlugged}
        <BatteryCharging size={20} color="var(--main)" />
      {:else if batteryLevel > 75}
        <BatteryFull size={20} color={batteryColor} />
      {:else if batteryLevel >= 35}
        <BatteryMedium size={20} color={batteryColor} />
      {:else}
        <BatteryLow size={20} color="red" />
      {/if}
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
  }

  .battery-text {
    font-size: 14px;
    font-weight: 500;
    color: var(--text-color-secondary);
    min-width: 50px;
    padding-left: 5px;
  }
</style>
