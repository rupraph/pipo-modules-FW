<script lang="ts">
  import { pipoio } from "../../pipoio";
  import type { GeneralConfig } from "../../types";
  import Text from "../form/Text.svelte";
  import Select from "../form/Select.svelte";
  import Switch from "../form/Switch.svelte";
  import Tooltip from "../tooltip/Tooltip.svelte";
  import Radio from "../form/Radio.svelte";
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

<Tooltip title="Toggling mode requires a reboot">
  <Radio
    label="Output Mode:"
    options={[
      { label: "MIDI (USB or BLE)", value: 1 },
      { label: "OSC (Wifi)", value: 0 },
    ]}
    value={generalconfig.MidiEnabled ? 1 : 0}
    on:change={(e) => {
      const isMidiMode = e.detail === 1;
      generalconfig.MidiEnabled = isMidiMode;
      generalconfig.OSC_ENA = !isMidiMode;
      // Disable BLE when switching to OSC mode
      if (!isMidiMode) {
        generalconfig.BLEEnabled = false;
      }
    }}
  />
  {#if generalconfig.MidiEnabled}
    <Switch
      label="Enable BLE "
      bind:value={generalconfig.BLEEnabled}
      design="slider"
    />
  {/if}
</Tooltip>
<Text
  label="Pipo Name"
  bind:value={generalconfig.PipoName}
  maxlength={schema.name.max}
  minlength={schema.name.min}
  {validate}
/>

<!-- <button class="primary" on:click={reboot} style="width: fit-content"
  >Reboot</button
> -->
