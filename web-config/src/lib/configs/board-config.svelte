<script lang="ts">
  import { pipoio } from "../../pipoio";
  import type { GeneralConfig } from "../../types";
  import Text from "../form/Text.svelte";
  import Select from "../form/Select.svelte";
  import Switch from "../form/Switch.svelte";
  import Tooltip from "../tooltip/Tooltip.svelte";
  import { schema } from "../../schema";

  export let generalconfig: GeneralConfig;
  const validate = (name) => {
    // firbid spaces
    if (name.includes(" ")) {
      return "Spaces are not allowed";
    }
    // forbid any other character than a-zA-Z0-9
    const regex = /^[a-zA-Z0-9]+$/;
    if (!regex.test(name)) {
      return "Only letters and numbers are allowed";
    }
    return "";
  };
  const options = [
    { label: "Acces Point", value: "AP" },
    { label: "Station", value: "STA" },
  ];
</script>

<!-- <Select label="Wifi Mode" {options} bind:value={generalconfig.Wifi_mode} /> -->

<Text
  label="Pipo Name"
  bind:value={generalconfig.PipoName}
  maxlength={schema.name.max}
  minlength={schema.name.min}
  {validate}
/>
<Tooltip title="Toggling BLE requires a reboot">
  <Switch
    label="Enable BLE "
    bind:value={generalconfig.BLEEnabled}
    design="slider"
  />
</Tooltip>

<!-- <button class="primary" on:click={reboot} style="width: fit-content"
  >Reboot</button
> -->
