<script lang="ts">
  import { pipoio } from "../../pipoio";
  import { isLive } from "../../services";
  import WifiConnect from "../wifi/connect.svelte";
  import { Wifi, WifiHigh } from "lucide-svelte";
  import Signal from "../wifi/signal.svelte";
  import Modal from "../modal.svelte";
  import { wifiState } from "../wifi/store";
  import { fetchNetworks, fetchState } from "../../services/wifi";
  import MenuButton from "./MenuButton.svelte";

  let wifiSignal = 4;
  let open = false;
  let wifiSsid = "";
  let apIP = "";
  let staIP = "";
  let disconnected = false;
  let isConnected = false;
  let live = false;
  isLive.subscribe((value) => {
    live = value;
  });
  wifiState.subscribe((value) => {
    wifiSignal = value.signal;
    wifiSsid = value.ssid === "none" ? "Not connected" : value.ssid;
    apIP = value.apIP;
    staIP = value.staIP;
    disconnected = value.status === "DISCONNECTED";
    isConnected = value.status === "CONNECTED";
  });

  $: fetchNetworks().then(() => fetchState());

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

<MenuButton connected={isConnected} on:click={() => (open = !open)}>
  <div class="icon">
    <WifiHigh color="var(--bg-primary)" size={30} strokeWidth={3} />
  </div>
</MenuButton>
<Modal bind:open>
  <WifiConnect />
</Modal>

<style>
  .icon {
    display: flex;
    align-items: center;
    justify-content: center;
    transform: translateY(-3px);
  }
</style>
