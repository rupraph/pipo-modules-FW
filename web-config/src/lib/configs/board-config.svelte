<script lang="ts">
  import Select from "../form/Select.svelte";
  import axios from "axios";

  export let wifiMode: string;

  function reboot() {
    axios.get("/reboot").then(() => {
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
