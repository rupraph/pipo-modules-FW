<script lang="ts">
  import type {
    AnalogOut,
    AnalogOutKeys,
    PipoConfig,
    PipoKeys,
  } from "../../types";
  type Keys = PipoKeys["analog"];
  import { currentConfig, currentMode } from "../../services/config";
  import { uiState } from "../ui-state";
  import TouchCalibration from "../TouchCalibration.svelte";

  let category: "analog" | "touch" = "analog";
  const boardType = "analog";

  // Initialize category from uiState or use default
  $: if ($uiState[boardType]?.channelType) {
    category = $uiState[boardType]!.channelType as "analog" | "touch";
  }

  // Update uiState when category changes
  $: if (category) {
    uiState.setChannelType(boardType, category);
    uiState.setSelectedChannel(boardType, keys[0]);
  }

  $: config = $currentConfig;
  $: mode = $currentMode;
  $: engineKey = (mode === "MIDI" ? "engine-midi" : "engine-osc") as
    | "engine-midi"
    | "engine-osc";
  $: keys =
    category === "analog"
      ? Array.from({ length: 8 }, (_, i) => `A0${i + 1}` as Keys)
      : Array.from({ length: 8 }, (_, i) => `T${i + 1}` as Keys);

  $: selectedChannel = $uiState[boardType]?.selectedChannel;

  // Set first channel as default if none selected OR if selected channel is not in current keys
  $: if (
    config &&
    keys.length > 0 &&
    (!selectedChannel || !keys.includes(selectedChannel as Keys))
  ) {
    uiState.setSelectedChannel(boardType, keys[0]);
  }

  // Create a reactive object that tracks enabled state for each channel
  // Force reactivity by also depending on config and currentConfig
  $: channelStates =
    config && $currentConfig
      ? keys.reduce(
          (acc, key) => {
            acc[key] = config.engine[engineKey][key].enabled ?? false;
            return acc;
          },
          {} as Record<Keys, boolean>
        )
      : ({} as Record<Keys, boolean>);

  function toggle(key: Keys) {
    if (!config) return;
    config.engine[engineKey][key].enabled =
      !config.engine[engineKey][key].enabled;
    console.log(config.engine[engineKey][key].enabled);
    currentConfig.set(config);
  }
  function isEnabled(key: Keys) {
    if (!config) return false;
    return config.engine[engineKey][key].enabled;
  }
  function selectChannel(key: Keys) {
    uiState.setSelectedChannel(boardType, key);
  }
</script>

{#if config}
  <div class="category">
    <button
      class:selected={category === "analog"}
      on:click={() => (category = "analog")}
    >
      Analog
    </button>
    <button
      class:selected={category === "touch"}
      on:click={() => (category = "touch")}>Touch</button
    >
  </div>
  <div class="channels">
    {#each keys as key}
      <button
        class:enabled={channelStates[key]}
        class:selected={key === selectedChannel}
        on:click={() => {
          selectChannel(key);
          console.log(`Selected channel: ${selectedChannel}`);
        }}
      >
        {key}
      </button>
    {/each}
  </div>
  {#if category === "touch"}
    <TouchCalibration />
  {/if}
{/if}

<style scoped>
  .category > button {
    width: 145px;
    height: 32px;
    color: var(--text-color);
    font-weight: 700;
    font-size: 14px;
    border: none;
    background-color: var(--bg-secondary);
    cursor: pointer;
    border-radius: 0px;
  }
  .category > button.selected {
    /* background-color: var(--main); */
    /* color: var(--bg-secondary); */
    outline: 3px solid var(--main);
    outline-offset: -3px;
  }
  .channels {
    display: grid;
    grid-template-columns: repeat(4, 64px);
    grid-template-rows: repeat(2, 48px);
    gap: 10px;
    margin-top: 15px;
  }
  .channels > button {
    width: 60px;
    height: 42px;
    background-color: var(--bg-secondary);
    border: none;
    color: var(--grey);
    font-family: Instrument Sans;
    font-weight: 700;
    font-size: 16px;
    cursor: pointer;
    padding: 0;
  }
  .channels > button.enabled {
    /* background-color: var(--main); */
    color: var(--main);
  }
  .channels > button.selected {
    outline: 6px solid var(--main);
    /* border-radius: 8px; */
    outline-offset: -6px;
  }
</style>
