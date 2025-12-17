<script lang="ts">
  import type { PipoKeys } from "../../types";
  type Keys = PipoKeys["motion"];
  import { currentConfig, currentMode } from "../../services/config";
  import { uiState } from "../ui-state";
  import PillSwitch from "../form/PillSwitch.svelte";
  import InfoModal from "../InfoModal.svelte";

  const channelTypes = [
    {
      label: "Euler Angles",
      value: "euler",
    },
    {
      label: "Lin Accell",
      value: "linear_acceleration",
    },

    { label: "Magneto", value: "magnitude" },
    {
      label: "Gyro",
      value: "angular_acceleration",
    },
    {
      label: "Quaternions",
      value: "quaternion",
    },
  ];

  let currentType = "euler";
  const boardType = "motion";

  // Initialize currentType from uiState or use default
  $: if ($uiState[boardType]?.channelType) {
    currentType = $uiState[boardType]!.channelType;
  }

  // Update uiState when currentType changes
  $: if (currentType) {
    uiState.setChannelType(boardType, currentType);
    uiState.setSelectedChannel(boardType, keys[0]);
  }

  $: config = $currentConfig;
  $: mode = $currentMode;
  $: engineKey = (mode === "MIDI" ? "engine-midi" : "engine-osc") as
    | "engine-midi"
    | "engine-osc";

  // Filter channel types based on mode - hide quaternion in MIDI mode
  $: availableChannelTypes = channelTypes.filter(
    (type) => !(mode === "MIDI" && type.value === "quaternion")
  );

  $: keys =
    currentType === "euler"
      ? (["yaw", "pitch", "roll"] as Keys[])
      : currentType === "linear_acceleration"
        ? (["accX", "accY", "accZ"] as Keys[])
        : currentType === "magnitude"
          ? (["magX", "magY", "magZ"] as Keys[])
          : currentType === "angular_acceleration"
            ? (["gyroX", "gyroY", "gyroZ"] as Keys[])
            : [];

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

  // Check if any channel in a category is enabled
  function isCategoryEnabled(categoryValue: string): boolean {
    if (!config) return false;

    if (categoryValue === "quaternion") {
      // For quaternion, check the special engine setting
      return config.engine["engine-special"]["quat"]?.enabled ?? false;
    }

    // For other categories, check if any channel in that category is enabled
    const categoryKeys =
      categoryValue === "euler"
        ? (["yaw", "pitch", "roll"] as Keys[])
        : categoryValue === "linear_acceleration"
          ? (["accX", "accY", "accZ"] as Keys[])
          : categoryValue === "magnitude"
            ? (["magX", "magY", "magZ"] as Keys[])
            : categoryValue === "angular_acceleration"
              ? (["gyroX", "gyroY", "gyroZ"] as Keys[])
              : [];

    return categoryKeys.some(
      (key) => config.engine[engineKey][key]?.enabled ?? false
    );
  }

  // Force reactivity for category enabled state
  $: categoryEnabledStates =
    config && $currentConfig
      ? availableChannelTypes.reduce(
          (acc, type) => {
            acc[type.value] = isCategoryEnabled(type.value);
            return acc;
          },
          {} as Record<string, boolean>
        )
      : {};

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

<span class="row" style="align-items: center; gap: 0.5em; ">
  <h3>Channel Settings</h3>
  <InfoModal>
    <p>
      Select a channel category to display underlying channels. The channels
      which are enabled have their name in yellow (Grey if disabled).
    </p>
    <p>
      <u>Euler angles:</u> are angular position around the three principal axes (X,
      Y, Z) representing roll, pitch, and yaw. (This are impactied by absolute or
      relative orientation setting. As well, this way of acquiring orientation is
      subject to gimbal lock)
    </p>
    <p>
      <u>Linear Accelleration:</u> is the change of velocity along the three principal
      axes (X, Y, Z) excluding the effect of gravity.
    </p>
    <p>
      <u>Magnetometer readings:</u> represent the magnetic field strength along the
      three principal axes (X, Y, Z)
    </p>
    <p>
      <u>Gyroscope readings:</u> represent the angular velocity around the three
      principal axes (X, Y, Z) measuring how fast the device is rotating.
    </p>
    <p>
      <u>Quaternions:</u> are a four-dimensional number system that provides a robust
      way to represent 3D orientations, avoiding issues like gimbal lock associated
      with Euler angles. Their output depends on absolute or relative mode setting.
    </p>
  </InfoModal>
</span>
{#if config}
  <div class="category">
    {#each availableChannelTypes as typeOption}
      <button
        class:selected={currentType === typeOption.value}
        class:enabled={categoryEnabledStates[typeOption.value]}
        on:click={() => (currentType = typeOption.value)}
      >
        {typeOption.label}
      </button>
    {/each}
  </div>
  {#if currentType === "quaternion"}
    <div class="row">
      <div class="label">Quaternion</div>
      <PillSwitch
        label=""
        bind:value={config.engine["engine-special"]["quat"].enabled}
      />
    </div>
  {:else}
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
    grid-template-columns: repeat(3, 64px);
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
    font-weight: 00;
    font-size: 14px;
    cursor: pointer;
    padding: 0;
  }
  .channels > button.enabled {
    /* background-color: var(--main); */
    color: var(--main);
    font-weight: 1000;
  }
  .channels > button.selected {
    outline: 6px solid var(--main);
    /* border-radius: 8px; */
    outline-offset: -6px;
  }
</style>
