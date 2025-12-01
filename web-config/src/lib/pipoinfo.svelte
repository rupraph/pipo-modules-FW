<script lang="ts">
  import { pipoio } from "../pipoio";
  import type { PipoInfo } from "../types";
  import { onMount, onDestroy } from "svelte";

  export let info: PipoInfo;

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
    // Fetch battery level immediately
    fetchBatteryLevel();

    // Set up interval to fetch battery level every 5 seconds
    intervalId = setInterval(fetchBatteryLevel, 5000);
  });

  onDestroy(() => {
    // Clear the interval when the component is destroyed
    clearInterval(intervalId);
  });
</script>

<div class="pipo-info">
  <span>MAC</span> <span>{info.mac}</span>
  <span>IP</span> <span>{info.ip}</span>
  <span>Type</span> <span>{info.type}</span>
  <span>Name</span> <span>{info.name}</span>
  <span>Version</span> <span>{info.version}</span>
  <span>Battery</span>
  {#if battPercentage === null && !isPlugged}
    <span>Waiting for Pipo to respond...</span>
  {:else if isPlugged}
    <span>Plugged</span>
  {:else}
    <span>{battPercentage} %</span>
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
