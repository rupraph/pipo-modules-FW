<script lang="ts">
  import { pipoio } from "../../pipoio";
  import { pipoType } from "../../services";
  import WifiConnect from "../wifi/connect.svelte";
  import Modal from "../modal.svelte";
  import Settings from "./Settings.svelte";
  import Wifi from "./Wifi.svelte";
  import PlayPause from "./PlayPause.svelte";
  import ConfigManagerButton from "./ConfigManagerButton.svelte";
  import { GraduationCap } from "lucide-svelte";
  import { tutorialService } from "../../tutorial/store";

  let wifiOpen = false;

  function startTutorial() {
    tutorialService.replay();
  }

  function fetchImage() {
    // debugger;
    return pipoio
      .get(`/${$pipoType}-Horizontal-Yellow.svg`, {
        responseType: "arraybuffer",
      })
      .then(({ data }) => {
        const blob = new Blob([data], { type: "image/svg+xml" });
        const url = URL.createObjectURL(blob);
        return url;
      });
  }
</script>

<section>
  {#await fetchImage()}
    <p></p>
  {:then imageDataUrl}
    <div class="row">
      <img src={imageDataUrl} alt="Pipo Logo" class="logo-image" />
      <div class="centered">
        <ConfigManagerButton />
        <span data-tutorial="menu-settings"><Settings /></span>
        <span data-tutorial="menu-wifi"><Wifi /></span>
        <PlayPause />
        <button
          class="tutorial-btn"
          on:click={startTutorial}
          title="Tutorial"
        >
          <GraduationCap size={18} color="var(--grey)" />
        </button>
      </div>
    </div>
  {/await}
</section>
<Modal bind:open={wifiOpen}>
  <WifiConnect />
</Modal>

<style>
  nav {
    width: 100%;
    display: flex;
    flex-wrap: wrap;
    /* padding: 0.5em; */
    gap: 1em;
    align-items: center;
    box-sizing: border-box;
    justify-content: space-between;
  }
  .controls {
    display: flex;
    gap: 1em;
    align-items: center;
  }
  .logo-image {
    width: 200px;
    height: auto;
  }
  .tutorial-btn {
    all: unset;
    cursor: pointer;
    display: flex;
    align-items: center;
    justify-content: center;
    width: 30px;
    height: 30px;
    border-radius: 50%;
    transition: background-color 0.15s;
  }
  .tutorial-btn:hover {
    background-color: var(--bg-secondary);
  }
</style>
