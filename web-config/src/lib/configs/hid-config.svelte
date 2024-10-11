<script lang="ts">
  import Select from "../form/Select.svelte";
  import Checkbox from "../form/Checkbox.svelte";
  import Text from "../form/Text.svelte";
  import type { SensorConfig, HidConfig } from "../../types";

  export let hidEnabled: boolean;
  export let hidMode: number;
  export let sensor: SensorConfig;
  export let hid: HidConfig;
</script>

<h4>Keyboard/Mouse mode settings</h4>
<Checkbox label="HID Enabled" bind:value={hidEnabled} />
<Select
  label="HID Mode"
  options={[
    { label: "Keyboard", value: 2 },
    { label: "Mouse", value: 1 },
  ]}
  bind:value={hidMode}
/>
<h4>Please restart Pipo after enabling or switching HID mode</h4>
<h4>
  NOTE: The available mapping options below will depend on the sensor and Hid
  mode
</h4>
{#if sensor.mode === true && hidMode === 2}
  <h4>
    Map a keyboard key. Address format for "u" would be: "KEY_u" (or
    KEY_UP,KEY_ENTER,...)
  </h4>
  <Checkbox label="Stroke continuous" bind:value={hid.stroke_mode} />
  <Text label="Address" bind:value={hid.addr} />
  {#if hid.stroke_mode && sensor.th_mode === true}
    <Text label="Address2" bind:value={hid.addr2} />
  {/if}
{:else if sensor.mode === true && hidMode === 1}
  <h4>Map a mouse button ("LEFT" or "RIGHT")</h4>
  <Checkbox label="Stroke continuous" bind:value={hid.stroke_mode} />
  <Text label="Address" bind:value={hid.addr} />
{:else if sensor.mode === false && hidMode === 2}
  <h4>
    Not possible to map a continuous sensor axis to a key stoke (must change to
    Threshold mode)
  </h4>
{:else if sensor.mode === false && hidMode === 1}
  <h4>
    Map a continuous sensor axis to a mouse axis (Address can be
    "X","Y","WHEEL","PAN")
  </h4>
  <Text label="Address" bind:value={hid.addr} />
{:else}
  PROBLEM !
{/if}
