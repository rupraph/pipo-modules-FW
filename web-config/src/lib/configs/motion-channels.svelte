<script lang="ts">
  import type { PipoKeys } from "../../types";
  type Keys = PipoKeys["motion"];
  import { currentConfig, currentMode } from "../../services/config";
  import { uiState } from "../ui-state";

  const channelTypes = [
    {
      label: "Lin Acc",
      value: "linear_acceleration",
    },
    {
      label: "Euler",
      value: "euler",
    },
    { label: "Mag", value: "magnitude" },
    {
      label: "Ang Acc",
      value: "angular_acceleration",
    },
    {
      label: "Quat",
      value: "quaternion",
    },
  ];

  let currentType = "euler";
  const boardType = "motion";

  $: config = $currentConfig;
  $: mode = $currentMode;
  $: engineKey = (mode === "MIDI" ? "engine-midi" : "engine-osc") as
    | "engine-midi"
    | "engine-osc";
  $: keys =
    currentType === "euler"
      ? (["yaw", "pitch", "roll"] as Keys[])
      : currentType === "linear_acceleration"
        ? (["accX", "accY", "accZ"] as Keys[])
        : currentType === "magnitude"
          ? (["magX", "magY", "magZ"] as Keys[])
          : currentType === "angular_acceleration"
            ? (["gyroX", "gyroY", "gyroZ"] as Keys[])
            : currentType === "quaternion"
              ? // TODO: Rup the quaternion is not defined in the types, I dont know
                // what it is supposed to be
                (["yaw", "pitch", "roll"] as Keys[])
              : [];

  $: selectedChannel = $uiState[boardType]?.selectedChannel;

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
    {#each channelTypes as typeOption}
      <button
        class:selected={currentType === typeOption.value}
        on:click={() => (currentType = typeOption.value)}
      >
        {typeOption.label}
      </button>
    {/each}
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
{/if}

<style scoped>
  .category {
    display: flex;
    flex-wrap: wrap;
    justify-content: space-evenly;
    gap: 10px 12px;
    max-width: 300px;
  }
  .category > button {
    width: 67px;
    height: 32px;
    color: var(--main);
    font-weight: 700;
    font-size: 14px;
    border: none;
    background-color: var(--bg-secondary);
    cursor: pointer;
    padding: 0;
  }
  .category > button.selected {
    background-color: var(--main);
    color: var(--bg-secondary);
  }
  .channels {
    display: grid;
    grid-template-columns: repeat(3, 64px);
    gap: 10px;
    margin-top: 15px;
  }
  .channels > button {
    width: 64px;
    height: 48px;
    background-color: var(--bg-secondary);
    border: none;
    color: var(--main);
    font: Instrument Sans;
    font-weight: 700;
    font-size: 16px;
    cursor: pointer;
    padding: 0;
  }
  .channels > button.enabled {
    background-color: var(--main);
    color: var(--bg-secondary);
  }
  .channels > button.selected {
    outline: 2px solid var(--main);
    outline-offset: 2px;
  }
</style>
