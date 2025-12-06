<script lang="ts">
  import { pipoio } from "../../pipoio";
  import { pipoType } from "../../services";
  import WifiConnect from "../wifi/connect.svelte";
  import Modal from "../modal.svelte";
  import Settings from "./Settings.svelte";
  import Wifi from "./Wifi.svelte";
  import PlayPause from "./PlayPause.svelte";

  let wifiOpen = false;

  function fetchImage() {
    // debugger;
    return pipoio
      .get(`/${$pipoType}-Horizontal-Yellow.svg`, { responseType: "arraybuffer" })
      .then(({ data }) => {
        const blob = new Blob([data], { type: "image/svg+xml" });
        const url = URL.createObjectURL(blob);
        return url;
      });
  }
</script>

<nav>
  {#await fetchImage()}
    <p></p>
  {:then imageDataUrl}
    <img src={imageDataUrl} alt="Pipo Logo" class="logo-image" />
    <Settings />
    <Wifi />
    <PlayPause />
  {/await}
</nav>
<Modal bind:open={wifiOpen}>
  <WifiConnect />
</Modal>

<style>
  nav {
    width: 100%;
    display: flex;
    flex-wrap: wrap;
    padding: 0.5em;
    gap: 1em;
    align-items: center;
    box-sizing: border-box;
  }
  .logo-image {
    width: 200px;
    height: auto;
  }
</style>
