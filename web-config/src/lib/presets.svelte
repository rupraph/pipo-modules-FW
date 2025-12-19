<script lang="ts">
  import {
    presets,
    presetsLoading,
    presetsService,
    type Preset,
  } from "../services/presets";
  import { onMount } from "svelte";
  import LoadingButton from "./form/LoadingButton.svelte";
  import Select from "svelte-select";
  import InfoModal from "./InfoModal.svelte";

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
      // Load the preset into current config
      // The floating save button will handle the actual save and reboot if needed
      await presetsService.applyPreset(selectedPreset.name);
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
        --font-size="16px"
        --color="var(--bg-primary)"
        --item-color="var(--text-color)"
        --item-bg="var(--bg-tertiary)"
        --item-is-active-color="var(--main-lighter)"
        --item-is-active-bg="var(--bg-secondary)"
        --input-color="var(--text-color-secondary)"
        --list-background="var(--bg-tertiary)"
        --item-hover-bg="var(--bg-tertiary)"
        --list-border="2px solid var(--text-color)"
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
</style>
