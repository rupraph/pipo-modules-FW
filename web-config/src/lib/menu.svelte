<script lang="ts">
  import { pipoio } from "../pipoio";
  import { isLive } from "../services";
  import WifiConnect from "./wifi/connect.svelte";
  import Signal from "./wifi/signal.svelte";
  import Modal from "./modal.svelte";
  import { wifiState } from "./wifi/store";

  let wifiSignal = 4;
  let wifiOpen = false;
  function toggleWifi() {
    wifiOpen = !wifiOpen;
  }
  let live = false;
  isLive.subscribe((value) => {
    live = value;
  });
  wifiState.subscribe((value) => {
    wifiSignal = value.signal;
  });
  let fps = 0;
  const max = 10;
  let last = 0;
  const dts: number[] = new Array(max).fill(0);
  pipoio
    .on("fps", ({ frames, dt }) => {
      dts[last++ % max] = frames / dt;
      fps = Math.round((dts.reduce((a, b) => a + b, 0) / max) * 1000);
    })
    .on("disconnect", () => {
      fps = 0;
    });
</script>

<nav>
  <span class="status {live ? 'live' : ''}"> </span>
  <span class="wifi" on:click={toggleWifi}>
    <Signal signal={wifiSignal} />Wi-Fi
  </span>
</nav>
<Modal bind:open={wifiOpen}>
  <WifiConnect />
</Modal>

<style>
  nav {
    width: 100%;
    display: flex;
    justify-content: flex-end;
    padding: 0.5em;
    gap: 1em;
    align-items: center;
    box-sizing: border-box;
  }
  .status {
    width: 1em;
    height: 1em;
    border-radius: 50%;
    background-color: var(--red);
    transition: all 1.5s;
    filter: drop-shadow(0 0 0.5em var(--red));
  }
  .status.live {
    background-color: var(--green);
    filter: drop-shadow(0 0 0.5em var(--green));
  }
  .wifi {
    cursor: pointer;
    height: 1em;
  }
</style>
