<script lang="ts">
  import Logs from "./lib/logs.svelte";
  import Toasts from "./lib/toast/toasts.svelte";
  import type { PipoInfo, PipoTypes } from "./types";
  import { pipoType, ip, pipoInfo } from "./services";
  import Collapse from "./lib/collapse.svelte";
  import Configs from "./lib/configs/index.svelte";
  import ConfigSelect from "./lib/configs/config-select.svelte";
  import { pipoio } from "./pipoio";
  import Menu from "./lib/menu/menu.svelte";
  import OfflineOverlay from "./lib/offline-overlay.svelte";
  import Pipoinfo from "./lib/pipoinfo.svelte";
  import AnalogChannels from "./lib/configs/analog-channels.svelte";
  import MotionChannels from "./lib/configs/motion-channels.svelte";
  import ChannelSettings from "./lib/configs/channel-settings.svelte";
  import FloatingSaveButton from "./lib/FloatingSaveButton.svelte";
  import BatteryStatus from "./lib/BatteryStatus.svelte";
  import MidiOutputSettings from "./lib/configs/midi-output-settings.svelte";
  import OscOutputSettings from "./lib/configs/osc-output-settings.svelte";
  import TouchCalibration from "./lib/TouchCalibration.svelte";
  import {
    currentConfig,
    hasUnsavedChanges,
    originalConfig,
  } from "./services/config";
  import { uiState } from "./lib/ui-state";
  import Presets from "./lib/presets.svelte";
  import PillSwitch from "./lib/form/PillSwitch.svelte";
  import InfoModal from "./lib/InfoModal.svelte";
  import Info from "./lib/icons/info.svelte";

  let type: PipoTypes = "unknown";
  let errorReloadTimeout: number | undefined;

  function fetch() {
    return pipoio
      .get<PipoInfo>("/info", { timeout: 5000 })
      .then(({ data, status, statusText }) => {
        type = data.type.toLowerCase().replace("pipo_", "") as PipoTypes;
        ip.set(data.ip);
        pipoType.set(type);
        pipoInfo.set(data); // Store globally for other components
        return data;
      })
      .catch((error) => {
        // Schedule page reload after 5 seconds if fetch fails
        // This ensures the page keeps trying to connect
        errorReloadTimeout = setTimeout(() => {
          console.log("Initial fetch failed - reloading page to retry");
          window.location.reload();
        }, 5000) as any as number;

        // Provide a more user-friendly error message
        throw new Error(
          "Unable to connect to Pipo. Please check if the device is powered on and WiFi is connected."
        );
      });
  }

  function handleSaveSuccess() {
    // Reset the original config to the current config after successful save
    if ($currentConfig) {
      originalConfig.set(JSON.parse(JSON.stringify($currentConfig)));
      hasUnsavedChanges.set(false);
    }
  }

  async function fetchRelativeModeState() {
    try {
      const response = await pipoio.get("/relative-mode");
      const isRelative =
        response.data === "true" ||
        response.data === true ||
        response.data === 1;
      // Update the current config with the actual state from the server
      if ($currentConfig?.sensorconf) {
        $currentConfig.sensorconf.relative_mode = isRelative;
        currentConfig.set($currentConfig);
      }
    } catch (error) {
      console.error("Failed to fetch relative mode state:", error);
    }
  }

  async function reset_orientation() {
    try {
      await pipoio.get("/setreference");
      console.log("Reference orientation reset");
      // Fetch actual state from backend to ensure sync
      await fetchRelativeModeState();
    } catch (error) {
      console.error("Failed to reset orientation:", error);
      // Revert to actual backend state on error
      await fetchRelativeModeState();
    }
  }
</script>

<main>
  <Toasts />
  {#await fetch()}
    <p>Waiting for Pipo to respond...</p>
  {:then resp}
    <div class="main-container">
      <BatteryStatus />
      <Menu />
      <!-- <ConfigSelect /> -->
      <section style="border-top: 2px solid var(--bg-tertiary);">
        {#if type !== "range"}
          {#if type === "motion"}
            <div
              class="row"
              style="border-bottom: 1px dashed var(--bg-secondary); padding-bottom: 6px;"
            >
              <div style="display: flex; align-items: center; gap: 0.5em;">
                <h3 style="margin: 0; white-space: nowrap;">Relative mode</h3>
                <InfoModal>
                  <p style="white-space: normal;">
                    Choose whether the sensor uses relative or absolute
                    orientation.
                  </p>
                  <p>
                    When relative orientation is "ON" the sensor measurements
                    are relative to a reference orientation you can capture (Use
                    the "Capture Reference" button or the function button on
                    Pipo). By opposition, absolute orientation will output
                    measurements which are referenced to the North and the
                    ground.
                  </p>
                  <p>This impacts the Euler angles and quaternions outputs.</p>
                </InfoModal>
              </div>
              {#if $currentConfig?.sensorconf}
                <div style="display: flex; gap: 8px; align-items: center;">
                  <PillSwitch
                    label=""
                    bind:value={$currentConfig.sensorconf.relative_mode}
                  />
                  <button
                    class="primary"
                    class:disabled={!$currentConfig.sensorconf.relative_mode}
                    disabled={!$currentConfig.sensorconf.relative_mode}
                    on:click={() => {
                      reset_orientation();
                    }}
                  >
                    Capture Reference
                  </button>
                </div>
              {/if}
            </div>
          {/if}

          {#if type === "analog"}
            <AnalogChannels />
          {:else if type === "motion"}
            <MotionChannels />
          {/if}
        {:else}
          <div class="row">
            <div
              style="display: flex; align-items: center; gap: 0.5em; white-space: nowrap;"
            >
              <h3 style="margin: 0; white-space: nowrap;">Hold mode</h3>
              <InfoModal>
                <p style="white-space: normal;">
                  "Hold" defines the sensor behaviour when an obstacle is
                  removed from the measuring range of the sensor: when hold is
                  on, the last value is maintained. If hold is off, the sensor
                  returns min or max (depending on Over mode position).
                </p>
              </InfoModal>
            </div>
            {#if $currentConfig?.sensorconf}
              <PillSwitch
                label=""
                bind:value={$currentConfig.sensorconf.hold_mode}
              />
            {/if}
          </div>
        {/if}
        {#if type !== "motion" || $uiState[type]?.channelType !== "quaternion"}
          <div class="channel-settings">
            <ChannelSettings />
          </div>
        {/if}
      </section>
      <section class="output-settings">
        <!-- Output settings based on board's general output mode -->
        {#if $currentConfig?.general.MidiEnabled}
          <MidiOutputSettings />
        {:else if $currentConfig?.general.OSC_ENA}
          <OscOutputSettings />
        {/if}
      </section>
      <section
        style="border-top: 2px solid var(--bg-tertiary);border-bottom: 2px solid var(--bg-tertiary);"
      >
        <Presets />
      </section>
      <section>
        <!-- <Collapse title="Info" collapseId="info">
          <Pipoinfo info={resp} />
        </Collapse> -->
        <div class="row" style="color: var(--bg-secondary);">
          <div class="left">
            <p>SW:</p>
            <span>{resp.version}</span>
          </div>
        </div>
      </section>
      <!-- <article class="content section-borders">
        <Logs />
      </article> -->
    </div>

    <!-- Floating Save Button -->
    <FloatingSaveButton
      config={$currentConfig}
      show={$hasUnsavedChanges}
      onSaveSuccess={handleSaveSuccess}
    />
  {:catch e}
    <article>
      <h3>Network error</h3>
      <p>{e}</p>
    </article>
  {/await}

  <OfflineOverlay />
</main>

<style>
  main {
    display: flex;
    flex-direction: column;
    justify-content: space-around;
    align-items: center;
    width: 100%;
    max-width: 600px;
    gap: 1em;
    position: relative;
  }

  .channel-settings {
    border-top: 1px dashed var(--bg-secondary);
    padding-top: 12px;
    gap: 12px;
    display: flex;
    flex-direction: column;
    width: 100%;
  }

  .output-settings {
    border-radius: 12px;
    border-top: 1px dashed var(--bg-secondary);
    width: 100%;
    background-color: var(--bg-secondary);
  }

  /* .title-container {
    position: relative;
    text-align: center;
    height: fit-content;
    margin-bottom: 30px;
  } */
</style>
