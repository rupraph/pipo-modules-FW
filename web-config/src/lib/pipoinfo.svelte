<script lang="ts">
  import { pipoio } from "../pipoio";
  import type { PipoInfo } from "../types";
  import { onMount, onDestroy } from "svelte";

  export let info: PipoInfo;

  let batt: number | null = null;
  let intervalId: number;

  async function fetchBatteryLevel() {
    try {
      const { data } = await pipoio.get("/battlevel", { timeout: 2000 });
      batt = data / 1000;
    } catch (error) {
      console.error("Failed to fetch battery level:", error);
    }
  }

  onMount(() => {
    // Fetch battery level immediately
    fetchBatteryLevel();

    // Set up interval to fetch battery level every 5 seconds
    intervalId = setInterval(fetchBatteryLevel, 5000);
  });

  onDestroy(() => {
    // Clear the interval when the component is destroyed
    clearInterval(intervalId);
  });

  function capValue(value: number, min: number, max: number) {
    return Math.max(min, Math.min(max, value));
  }
</script>

<div class="pipo-info">
  <span>MAC</span> <span>{info.mac}</span>
  <span>IP</span> <span>{info.ip}</span>
  <span>Type</span> <span>{info.type}</span>
  <span>Name</span> <span>{info.name}</span>
  <span>Version</span> <span>{info.version}</span>
  <span>Battery</span>
  {#if batt === null}
    <span>Waiting for Pipo to respond...</span>
  {:else if batt >= 4.3}
    <span> Plugged</span>
  {:else}
    <span>{batt.toFixed(1)} V</span>
    <!-- <span><bold>Batt Voltage: </bold>{batt} V</span> -->
    <span>Batt Level:</span>
    <!--  100 = 4.1v, 0 = 3.3v => batt * 125 - 412.5 -->
    <!-- 100 =4.05v, 0 = 3,3 => batt * 133.3 - 439.8 -->
    <span>{capValue(batt * 133.3 - 439.8, 0, 100).toFixed(1)} %</span>
  {/if}
</div>

<style>
  .pipo-info {
    margin: 1rem;
    font-size: 1.1em;
    display: grid;
    grid-template-columns: auto auto;
    grid-template-columns: 7em auto;
    grid-auto-flow: column;
    justify-items: start;
    width: 100%;
    grid-auto-flow: row;
    width: 100%;
    column-gap: 1em;
  }
  .pipo-info > span:nth-child(2n + 1) {
    font-weight: bold;
  }
</style>
