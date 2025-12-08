<script lang="ts">
  import Logs from "./lib/logs.svelte";
  import Toasts from "./lib/toast/toasts.svelte";
  import type { PipoInfo, PipoTypes } from "./types";
  import { pipoType, ip } from "./services";
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
    currentMode,
    hasUnsavedChanges,
    originalConfig,
  } from "./services/config";
  import Presets from "./lib/presets.svelte";

  let type: PipoTypes = "unknown";
  function fetch() {
    return pipoio
      .get<PipoInfo>("/info", { timeout: 5000 })
      .then(({ data, status, statusText }) => {
        type = data.type.toLowerCase().replace("pipo_", "") as PipoTypes;
        ip.set(data.ip);
        pipoType.set(type);
        return data;
      });
  }

  function handleSaveSuccess() {
    // Reset the original config to the current config after successful save
    if ($currentConfig) {
      originalConfig.set(JSON.parse(JSON.stringify($currentConfig)));
      hasUnsavedChanges.set(false);
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
      <ConfigSelect />
      <section>
        <h3>Channel Settings</h3>
        {#if type === "analog"}
          <AnalogChannels />
        {:else if type === "motion"}
          <MotionChannels />
        {/if}
        <ChannelSettings />

        <!-- Output settings based on board's general output mode -->
        {#if $currentConfig?.general.MidiEnabled}
          <MidiOutputSettings />
        {:else if $currentConfig?.general.OSC_ENA}
          <OscOutputSettings />
        {/if}
      </section>
      <Presets />
      <article class="content section-borders">
        <Collapse title="Info" collapseId="info">
          <Pipoinfo info={resp} />
        </Collapse>
      </article>
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
    max-width: 600px;
    gap: 1em;
  }

  /* .title-container {
    position: relative;
    text-align: center;
    height: fit-content;
    margin-bottom: 30px;
  } */
</style>
