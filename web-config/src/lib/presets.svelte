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
      <span>Preset</span>
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
        class="action-btn"
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
