<script lang="ts">
  import Logs from "./lib/logs.svelte";
  import Toasts from "./lib/toast/toasts.svelte";
  import type { PipoInfo, PipoTypes } from "./types";
  import { pipoType, ip } from "./services";
  import Collapse from "./lib/collapse.svelte";
  import Configs from "./lib/configs/index.svelte";
  import { pipoio } from "./pipoio";
  import Menu from "./lib/menu.svelte";
  import OfflineOverlay from "./lib/offline-overlay.svelte";
  import { onMount, onDestroy } from "svelte";
  import Pipoinfo from "./lib/pipoinfo.svelte";

  let type: PipoTypes = "unknown";
  function fetch() {
    return pipoio
      .get<PipoInfo>("/info", { timeout: 5000 })
      .then(({ data, status, statusText }) => {
        type = data.type.toLowerCase().replace("pipo_", "") as PipoTypes;
        ip.set(data.ip);
        pipoType.set(type);
        return data;
      });
  }
  function fetchImage() {
    // debugger;
    return pipoio
      .get(`/pattern-${type}.svg`, { responseType: "arraybuffer" })
      .then(({ data }) => {
        const blob = new Blob([data], { type: "image/svg+xml" });
        const url = URL.createObjectURL(blob);
        return url;
      });
  }
</script>

<main>
  <Toasts />
  <Menu />
  {#await fetch()}
    <p>Waiting for Pipo to respond...</p>
  {:then resp}
    <div class="title-container">
      <h1>Pipo {type}</h1>
      {#await fetchImage()}
        <p>Loading image...</p>
      {:then imageDataUrl}
        <!-- Use the imageDataUrl as the src for the image -->
        <img src={imageDataUrl} alt="Pattern" class="pattern-image" />
      {/await}
    </div>
    <Configs />
    <article class="content section-borders">
      <Collapse title="Info">
        <Pipoinfo info={resp} />
      </Collapse>
    </article>
    <article class="content section-borders">
      <Logs />
    </article>
  {:catch e}
    <article>
      <h3>Network error</h3>
      <p>{e}</p>
    </article>
  {/await}

  <OfflineOverlay />
</main>

<style>
  main {
    display: flex;
    flex-direction: column;
    justify-content: space-around;
    align-items: center;
    max-width: 600px;
    gap: 1em;
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
    color: rgb(60, 60, 60); /* Adjust the text color for better visibility */
  }
</style>
