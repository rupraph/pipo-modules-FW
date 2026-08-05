<script lang="ts">
  import type { PipoKeys } from "../../types";
  type Keys = PipoKeys["max30102"];
  import { currentConfig, currentMode } from "../../services/config";
  import { uiState } from "../ui-state";

  const categories = [
    { label: "IR", value: "ir" },
    { label: "Red", value: "red" },
    { label: "Other", value: "other" },
  ];

  let category: string = "ir";
  const boardType = "max30102";

  // Initialize category from uiState or use default
  $: if ($uiState[boardType]?.channelType) {
    category = $uiState[boardType]!.channelType;
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
    category === "ir"
      ? (["ir_ac", "ir_raw", "ir_dc"] as Keys[])
      : category === "red"
        ? (["red_ac", "red_raw", "red_dc"] as Keys[])
        : (["hr_bpm", "temperature"] as Keys[]);

  $: selectedChannel = $uiState[boardType]?.selectedChannel;

  // Set first channel as default if none selected
  $: if (
    config &&
    keys.length > 0 &&
    (!selectedChannel || !keys.includes(selectedChannel as Keys))
  ) {
    uiState.setSelectedChannel(boardType, keys[0]);
  }

  // Track enabled state for each channel
  $: channelStates =
    config && $currentConfig
      ? keys.reduce(
          (acc, key) => {
            acc[key] = config.engine[engineKey][key].enabled ?? false;
            return acc;
          },
          {} as Record<string, boolean>,
        )
      : ({} as Record<string, boolean>);

  function isCategoryEnabled(cat: string): boolean {
    if (!config) return false;
    const catKeys =
      cat === "ir"
        ? ["ir_ac", "ir_raw", "ir_dc"]
        : cat === "red"
          ? ["red_ac", "red_raw", "red_dc"]
          : ["hr_bpm", "temperature"];
    return catKeys.some(
      (key) => config.engine[engineKey][key as Keys]?.enabled ?? false,
    );
  }

  // Use reduce with explicit Record type so the template can index by string
  $: categoryEnabledStates =
    config && $currentConfig
      ? categories.reduce(
          (acc, cat) => {
            acc[cat.value] = isCategoryEnabled(cat.value);
            return acc;
          },
          {} as Record<string, boolean>,
        )
      : ({} as Record<string, boolean>);
</script>

<div class="channel-list">
  <!-- Category tabs -->
  <div class="category">
    {#each categories as cat}
      <button
        class:selected={category === cat.value}
        class:enabled={categoryEnabledStates[cat.value]}
        on:click={() => (category = cat.value)}
      >
        {cat.label}
      </button>
    {/each}
  </div>

  <!-- Channel tabs -->
  <div class="channels">
    {#each keys as key}
      <button
        class:enabled={channelStates[key]}
        class:selected={key === selectedChannel}
        on:click={() => uiState.setSelectedChannel(boardType, key)}
      >
        {key}
      </button>
    {/each}
  </div>
</div>

<style>
  .channel-list {
    margin-bottom: 8px;
  }
  .category {
    display: flex;
    flex-wrap: wrap;
    justify-content: space-evenly;
    gap: 10px 12px;
    max-width: 300px;
  }
  .category > button {
    width: 130px;
    height: 32px;
    color: var(--text-color);
    font-family: Instrument Sans;
    font-weight: 700;
    font-size: 14px;
    border: none;
    background-color: var(--bg-secondary);
    cursor: pointer;
    padding: 0;
    border-radius: 0px;
  }
  .category > button.enabled {
    color: var(--main);
  }
  .category > button.selected {
    outline: 3px solid var(--main);
    outline-offset: -3px;
  }
  .channels {
    display: grid;
    grid-template-columns: repeat(auto-fill, 64px);
    gap: 10px;
    margin-top: 15px;
  }
  .channels > button {
    width: 64px;
    height: 48px;
    background-color: var(--bg-secondary);
    border: none;
    color: var(--grey);
    font-family: Instrument Sans;
    font-weight: 400;
    font-size: 14px;
    cursor: pointer;
    padding: 4px 6px;
    box-sizing: border-box;
  }
  .channels > button.enabled {
    color: var(--main);
    font-weight: 1000;
  }
  .channels > button.selected {
    outline: 6px solid var(--main);
    outline-offset: -6px;
  }
</style>
