<script lang="ts">
  import type { PipoKeys } from "../../types";
  type Keys = PipoKeys["max30102"];
  import { currentConfig, currentMode } from "../../services/config";
  import { uiState } from "../ui-state";

  const categories = [
    { label: "Preconditioned", value: "preconditioned" },
    { label: "Raw", value: "raw" },
    { label: "Computed", value: "computed" },
  ];

  let category: string = "preconditioned";
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
    category === "preconditioned"
      ? (["ir_ac", "red_ac", "ir_dc", "red_dc"] as Keys[])
      : category === "raw"
        ? (["ir_raw", "red_raw", "temperature"] as Keys[])
        : (["hr_bpm"] as Keys[]);

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
      cat === "preconditioned"
        ? ["ir_ac", "red_ac", "ir_dc", "red_dc"]
        : cat === "raw"
          ? ["ir_raw", "red_raw", "temperature"]
          : ["hr_bpm"];
    return catKeys.some(
      (key) => config.engine[engineKey][key as Keys]?.enabled ?? false,
    );
  }

  $: categoryEnabledStates =
    config && $currentConfig
      ? {
          preconditioned: isCategoryEnabled("preconditioned"),
          raw: isCategoryEnabled("raw"),
          computed: isCategoryEnabled("computed"),
        }
      : { preconditioned: false, raw: false, computed: false };
</script>

<div class="channel-list">
  <!-- Category tabs -->
  <div class="row">
    {#each categories as cat}
      <button
        class="primary"
        class:active={category === cat.value}
        on:click={() => (category = cat.value)}
      >
        {cat.label}
        {#if categoryEnabledStates[cat.value]}
          <span class="enabled-dot" />
        {/if}
      </button>
    {/each}
  </div>

  <!-- Channel tabs -->
  <div class="row" style="margin-top: 8px;">
    {#each keys as key}
      <button
        class="primary"
        class:active={selectedChannel === key}
        on:click={() => uiState.setSelectedChannel(boardType, key)}
      >
        {key}
        {#if channelStates[key]}
          <span class="enabled-dot" />
        {/if}
      </button>
    {/each}
  </div>
</div>

<style>
  .channel-list {
    margin-bottom: 8px;
  }
  .row {
    display: flex;
    flex-wrap: wrap;
    gap: 4px;
  }
  button {
    position: relative;
    padding: 4px 10px;
    font-size: 0.85em;
  }
  button.active {
    background: var(--accent);
    color: white;
  }
  .enabled-dot {
    display: inline-block;
    width: 6px;
    height: 6px;
    border-radius: 50%;
    background: var(--success, #4caf50);
    margin-left: 4px;
    vertical-align: middle;
  }
</style>
