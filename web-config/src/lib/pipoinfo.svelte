<script lang="ts">
  import { pipoio } from "../pipoio";
  import type { PipoInfo } from "../types";

  export let info: PipoInfo;
  const batt = pipoio.get("/battlevel", { timeout: 2000 }).then(({ data }) => {
    return data / 1000;
  });
</script>

<div class="pipo-info">
  <span>MAC</span> <span>{info.mac}</span>
  <span>IP</span> <span>{info.ip}</span>
  <span>Type</span> <span>{info.type}</span>
  <span>Name</span> <span>{info.name}</span>
  <span>Version</span> <span>{info.version}</span>
  <span>Battery</span>
  {#await batt}
    <span>Waiting for Pipo to respond...</span>
  {:then resp}
    <span>{resp} V</span>
  {/await}
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
