<script lang="ts">
  import {
    presets,
    presetsLoading,
    presetsError,
    presetsService,
    type Preset,
  } from "../services/presets";
  import { onMount } from "svelte";
  import LoadingButton from "./form/LoadingButton.svelte";
  import Select from "svelte-select";
  import InfoModal from "./InfoModal.svelte";
  import { addToast } from "./toast";
  import { pipoio } from "../pipoio";
  import { currentConfig, originalConfig } from "../services/config";
  import { get } from "svelte/store";

  let selectedPreset: Preset | null = null;
  let applying = false;

  onMount(() => {
    presetsService.fetchPresets();
  });

  $: selectItems = $presets.map((preset) => ({
    value: preset.filename,
    label: preset.name,
  }));

  $: selectedItem = selectedPreset
    ? { value: selectedPreset.filename, label: selectedPreset.name }
    : null;

  async function handleSelect(event: CustomEvent) {
    const selected = event.detail;
    if (selected) {
      selectedPreset =
        $presets.find((p) => p.filename === selected.value) || null;
    }
  }

  async function handleLoad() {
    if (!selectedPreset) return;

    applying = true;

    try {
      // Get the original mode before loading preset
      const originalMode = get(originalConfig)?.general.MidiEnabled
        ? "MIDI"
        : "OSC";

      // Apply the preset
      await presetsService.applyPreset(selectedPreset.name);

      // Get the new mode after loading preset
      const newMode = get(currentConfig)?.general.MidiEnabled ? "MIDI" : "OSC";

      // Check if mode changed
      if (originalMode !== newMode) {
        // Show countdown toast
        addToast({
          type: "error",
          message:
            "The preset changed the MIDI/OSC mode. The board needs to reboot. Please reload the page in a few seconds.(Make sure Wifi is reconnected)",
          timeout: 10000,
        });

        // Trigger reboot after 5 seconds
        setTimeout(() => {
          pipoio
            .get("/reboot")
            .then(() => {
              console.log("Rebooting device...");
            })
            .catch((err) => {
              console.error("Failed to reboot:", err);
            });
        }, 5000);
      }
    } catch (err) {
      // Error is handled by the service and stored in presetsError
    } finally {
      applying = false;
    }
  }
</script>

<section class="preset-select">
  <div class="row">
    <div class="left">
      <span>Factory Presets</span>
      <InfoModal>
        <p>
          This loads a factory preset, applies and saves it to the current
          configuration.
        </p></InfoModal
      >
    </div>
    <div>
      <Select
        --height="25px"
        --max-height="25px"
        --width="134px"
        --padding="0"
        --value-container-padding="0"
        --selected-item-padding="0 0 0 10px"
        --selected-item-color="var(--bg-primary)"
        --font-size="14px"
        --color="var(--bg-primary)"
        --item-color="var(--bg-primary)"
        --item-bg="var(--bg-secondary)"
        --item-is-active-color="var(--main)"
        --item-is-active-bg="var(--bg-secondary)"
        --item-hover-color="var(--text-color)"
        --item-hover-bg="var(--bg-secondary)"
        --input-color="var(--text-color)"
        items={selectItems}
        value={selectedItem}
        on:change={handleSelect}
        placeholder={$presetsLoading ? "Loading..." : "Select a preset"}
        clearable={false}
        searchable={false}
        disabled={$presetsLoading}
      />
    </div>
    <div>
      {#if selectedPreset}
        <InfoModal>
          {selectedPreset.description || "No description available"}
        </InfoModal>
      {/if}
    </div>

    <div class="actions">
      <LoadingButton
        class="primary"
        loading={applying}
        disabled={!selectedPreset || $presetsLoading}
        onClick={handleLoad}
        title="Load preset"
        width="auto"
      >
        Load
      </LoadingButton>
    </div>

    {#if $presetsError}
      <div class="error-message">
        {$presetsError}
      </div>
    {/if}
  </div>
</section>

<style>
  section {
    width: 100%;
  }

  /* .row {
    display: flex;
    justify-content: space-between;
    align-items: center;
    width: 100%;
    gap: 1em;
  } */
  .row span {
    font-size: 14px;
    line-height: 14px;
  }

  .left span {
    font-size: 14px;
    font-weight: 500;
  }

  .select-container {
    flex: 1;
  }

  .actions {
    display: flex;
    gap: 8px;
    align-items: center;
  }

  .error-message {
    margin-top: 8px;
    padding: 8px 12px;
    background-color: var(--bg-primary);
    border: 1px solid var(--red);
    border-radius: 8px;
    color: var(--red);
    font-size: 13px;
  }
</style>
