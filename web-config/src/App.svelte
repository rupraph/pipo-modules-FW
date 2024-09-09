<script lang="ts">
  import Logs from "./lib/logs.svelte";
  import type { PipoTypes } from "./types";
  import { pipoType } from "./services";
  import Collapse from "./lib/collapse.svelte";
  import WifiConnect from "./lib/wifi/connect.svelte";
  import Configs from "./lib/configs/index.svelte";
  import axios, { AxiosError } from "axios";
  import Checkbox from "./lib/form/Checkbox.svelte";

  // import Piano from "./lib/vis/Piano.svelte";

  // this likely causes slow load as it loads image first. -> "eager"
  // only the image from the right type should be loaded by the client
  // (for later: also, while we migh want to keep all images when running in dev to develop on al three pipo, only the right image could be uploaded to the board.
  const images = import.meta.glob("./assets/pattern-*.svg", {
    eager: true,
    as: "url",
  });

  console.log("Images:", images);
  let type: PipoTypes = "unknown";
  let error: string;
  function onError(e: AxiosError) {
    if (!e.config || !e.config.url) {
      error = `Error fetching info: ${e}`;
      console.error("Error fetching info:", e);
      return;
    }
    const url = new URL(e.config.url);
    if (
      e.code === "ECONNABORTED" ||
      e.code === "ERR_ADDRESS_UNREACHABLE" ||
      e.code === "ERR_NETWORK"
    ) {
      error = `Error fetching ${url.pathname}: Maybe Pipo is not connected to the network?
      \nPlease connect Pipo to the network and refresh the page.\n
      Tried to from ${url.origin}`;
    } else {
      error = `Error fetching info: ${e}`;
    }
    console.error("Error fetching info:", e);
  }

  axios.get("/wifi-status").then(({ data }) => {
    console.log("Wifi status:", data);
  });
  axios.get("/wifi-networks").then(({ data }) => {
    console.log("Wifi networks:", data);
  });
  axios.get("/wifi-networks").then(({ data }) => {
    console.log("Wifi networks:", data);
  });
  const info = axios
    .get("/info", { timeout: 2000 })
    .then(({ data, status, statusText }) => {
      type = data.type.toLowerCase().replace("pipo_", "");
      console.log("Pipo type:", type);
      pipoType.set(type);
      return data;
    })
    .catch((e) => onError(e));
  $: PatternUrl = type ? `/assets/pattern-${type}.svg` : `/sheep.jpg`;

  function reboot() {
    axios.get("/reboot").then(() => {
      console.log("Rebooting...");
    });
  }
</script>

<main>
  <WifiConnect />
  <div class="title-container">
    <h1>Pipo {type}</h1>
    <img
      src={images[`./assets/pattern-${type}.svg`]}
      alt="Pattern"
      class="pattern-image"
    />
  </div>

  {#if error}
    <p class="error">{error}</p>
  {/if}
  <Configs />

  <article>
    <Logs />
  </article>
  {#if !error && info}
    {#await info}
      <p>Waiting for Pipo to respond...</p>
    {:then resp}
      <article>
        <Collapse title="Info" class="info">
          <span><bold>MAC</bold>{resp.mac}</span>
          <span><bold>IP</bold>{resp.ip}</span>
          <span><bold>Type</bold>{resp.type}</span>
          <span><bold>Name</bold>{resp.name}</span>
          <span><bold>Version</bold>{resp.version}</span>
        </Collapse>
        <!-- <Piano /> -->
      </article>
    {/await}
  {/if}
</main>

<style>
  main {
    display: flex;
    flex-direction: column;
    justify-content: space-around;
    align-items: center;
    max-width: 600px;
  }

  .pattern-image {
    position: absolute;
    top: 50%;
    left: 50%;
    transform: translate(-50%, -50%);
    height: 100%;
    object-fit: cover;
    z-index: -1; /* Ensure the image is behind the title */
  }

  .title-container {
    position: relative;
    text-align: center;
    height: fit-content;
    margin-bottom: 30px;
  }

  h1 {
    font-family: Brugty;
    position: relative;
    z-index: 1; /* Ensure the title is above the image */
    color: white; /* Adjust the text color for better visibility */
  }

  :global(.info) span {
    font-size: 1.1em;
    display: grid;
    grid-template-columns: 6em auto;
    grid-auto-flow: column;
    justify-items: start;
  }
</style>
