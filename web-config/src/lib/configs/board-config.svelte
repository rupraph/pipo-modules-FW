<script lang="ts">
  import { pipoio } from "../../pipoio";
  import Select from "../form/Select.svelte";

  export let wifiMode: string;

  function reboot() {
    pipoio.get("/reboot").then(() => {
      console.log("Rebooting...");
    });
  }
  const options = [
    { label: "Acces Point", value: "AP" },
    { label: "Station", value: "STA" },
  ];
</script>

<Select label="Wifi Mode" {options} bind:value={wifiMode} />
<button class="primary" on:click={reboot} style="width: fit-content"
  >Reboot</button
>
