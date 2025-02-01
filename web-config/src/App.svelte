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
  let info = fetch();
  pipoio.on("connect", () => {
    info = fetch();
  });
  // TODO: do this via websockets
  const batt = pipoio.get("/battlevel", { timeout: 2000 }).then(({ data }) => {
    return data / 1000;
  });
</script>

<main>
  <Toasts />
  <Menu />
  <div class="title-container">
    <h1>Pipo {type}</h1>
    <img
      src={`./assets/pattern-${type}.svg`}
      alt="Pattern"
      class="pattern-image"
    />
  </div>

  <Configs />
  <article>
    <Logs />
  </article>
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
        {#if batt}
          {#await batt}
            <p>Waiting for Pipo to respond...</p>
          {:then resp}
            <span><bold>Batt Voltage: </bold>{resp} V</span>
          {/await}
        {/if}
      </Collapse>
    </article>
  {:catch e}
    <article>
      <h3>Network error</h3>
      <p>Maybe pipo is not connected to WiFi?</p>
      <p>
        Please try to connect to the Pipo network, check if the problem
        persists.
      </p>
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

  :global(.info) span {
    font-size: 1.1em;
    display: grid;
    grid-template-columns: 6em auto;
    grid-auto-flow: column;
    justify-items: start;
  }
</style>
