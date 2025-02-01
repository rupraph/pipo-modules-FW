<script lang="ts">
  import Select from "../form/Select.svelte";
  import Checkbox from "../form/Checkbox.svelte";
  import Text from "../form/Text.svelte";
  import type { SensorConfig, HidConfig } from "../../types";
  import DisabledWarning from "./disabled-warning.svelte";
  export let hidMode: number;
  export let sensor: SensorConfig;
  export let hid: HidConfig;

  const modes = [
    { label: "Mouse", value: 1 },
    { label: "Keyboard", value: 2 },
  ];
  $: currentMode = modes.find((mode) => mode.value === hidMode);

  let KB_map_type = "Special Key";
  let KB_map_type2 = "Special Key";

  const KB_map_types = [
    { label: "Character", value: "Character" },
    { label: "Special Key", value: "Special Key" },
  ];

  const KB_special_keys = [
    { label: "UP", value: "KEY_UP" },
    { label: "DOWN", value: "KEY_DOWN" },
    { label: "LEFT", value: "KEY_LEFT" },
    { label: "RIGHT", value: "KEY_RIGHT" },
    { label: "ENTER", value: "KEY_ENTER" },
    { label: "ESC", value: "KEY_ESC" },
    { label: "SPACE", value: "KEY_SPACE" },
  ];

  const Mouse_buttons = [
    { label: "LEFT", value: "LEFT" },
    { label: "RIGHT", value: "RIGHT" },
  ];

  const Mouse_axes = [
    { label: "X", value: "X" },
    { label: "Y", value: "Y" },
    { label: "WHEEL", value: "WHEEL" },
    { label: "PAN", value: "PAN" },
  ];
</script>

<h4>Current HID mode is {currentMode ? currentMode.label : "Unknown"}</h4>
<!-- <p>
  NOTE: The available mapping options below will depend on the sensor and Hid
  mode
</p> -->

<!-- Thresh + keyboard -->
{#if sensor.mode === true && hidMode === 2}
  <!-- <h4>
    Map a keyboard key. Address format for "u" would be: "KEY_u" (or
    KEY_UP,KEY_ENTER,...)
  </h4> -->
  <!-- <Text label="Current map" value={hid.addr} readonly /> -->
  <h4>Map a key when true</h4>
  <Select
    label="Choose mapping type"
    options={KB_map_types}
    bind:value={KB_map_type}
  />
  {#if KB_map_type === "Character"}
    <Text label="Map to char" bind:value={hid.addr} />
  {:else}
    <Select label="Map to" options={KB_special_keys} bind:value={hid.addr} />
  {/if}
  <Checkbox label="Stroke mode" bind:value={hid.stroke_mode} />
  {#if hid.stroke_mode && sensor.th_mode === false}
    <h4>Map a key when false</h4>
    <Select
      label="Choose mapping type"
      options={KB_map_types}
      bind:value={KB_map_type2}
    />
    {#if KB_map_type2 === "Character"}
      <Text label="Map to char" bind:value={hid.addr2} />
    {:else}
      <Select
        label="Map to key"
        options={KB_special_keys}
        bind:value={hid.addr2}
      />
    {/if}
  {/if}
{:else if sensor.mode === false && hidMode === 2}
  <p>
    Not possible to map a sensor in continuous mode to a key stoke (must change
    to Threshold mode)
  </p>
  <!-- Thresh + mouse -->
{:else if sensor.mode === true && hidMode === 1}
  <!-- <Checkbox label="Stroke continuous" bind:value={hid.stroke_mode} /> -->
  <Select
    label="Map to mouse button"
    options={Mouse_buttons}
    bind:value={hid.addr}
  />
  <p>Note: to map to mouses axis, switch sensor to continuous</p>
{:else if sensor.mode === false && hidMode === 1}
  <Select
    label="Map to mouse axis"
    options={Mouse_axes}
    bind:value={hid.addr}
  />
  <p>Note: to map to mouses buttons, switch sensor to threshold</p>
{:else}
  PROBLEM !
{/if}

<DisabledWarning enabled={!hid.enabled} feature="HID" />
