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

  let selectedPreset: Preset | null = null;
  let applying = false;
  let successMessage = "";

  onMount(() => {
    presetsService.fetchPresets();
  });

  async function handleApply() {
    if (!selectedPreset) return;

    applying = true;
    successMessage = "";

    try {
      await presetsService.applyPreset(selectedPreset.name);
      successMessage = `Successfully applied preset: ${selectedPreset.name}`;
      setTimeout(() => {
        successMessage = "";
      }, 3000);
    } catch (err) {
      // Error is handled by the service and stored in presetsError
    } finally {
      applying = false;
    }
  }

  function handleRefresh() {
    presetsService.refresh();
  }

  function selectPreset(preset: Preset) {
    selectedPreset = preset;
  }
</script>

<div class="presets-container">
  <div class="presets-header">
    <h2>Presets</h2>
    <LoadingButton
      class="secondary"
      loading={$presetsLoading}
      onClick={handleRefresh}
      width="120px"
    >
      Refresh
    </LoadingButton>
  </div>

  {#if $presetsError}
    <div class="error-message">
      <p>Error: {$presetsError}</p>
    </div>
  {/if}

  {#if successMessage}
    <div class="success-message">
      <p>{successMessage}</p>
    </div>
  {/if}

  {#if $presetsLoading && $presets.length === 0}
    <div class="loading">
      <p>Loading presets...</p>
    </div>
  {:else if $presets.length === 0}
    <div class="empty-state">
      <p>No presets available</p>
    </div>
  {:else}
    <div class="presets-grid">
      {#each $presets as preset (preset.filename)}
        <div
          class="preset-card"
          class:selected={selectedPreset?.filename === preset.filename}
          on:click={() => selectPreset(preset)}
          role="button"
          tabindex="0"
          on:keydown={(e) => {
            if (e.key === "Enter" || e.key === " ") {
              selectPreset(preset);
            }
          }}
        >
          <h3>{preset.name}</h3>
          <p class="description">{preset.description}</p>
        </div>
      {/each}
    </div>

    <div class="actions">
      <LoadingButton
        class="success"
        loading={applying}
        disabled={!selectedPreset}
        onClick={handleApply}
        width="150px"
      >
        Apply Preset
      </LoadingButton>
    </div>
  {/if}
</div>

<style>
  .presets-container {
    width: 100%;
  }

  .presets-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 1.5rem;
  }

  .presets-header h2 {
    margin: 0;
    font-size: 1.5rem;
    font-weight: 600;
  }

  .error-message,
  .success-message {
    padding: 0.75rem 1rem;
    border-radius: 3px;
    margin-bottom: 1rem;
  }

  .error-message {
    background-color: var(--bg-primary);
    border: 2px solid var(--red);
    color: var(--red);
  }

  .success-message {
    background-color: var(--bg-primary);
    border: 2px solid var(--green);
    color: var(--green);
  }

  .loading,
  .empty-state {
    text-align: center;
    padding: 3rem 1rem;
    color: var(--text-color);
    opacity: 0.7;
  }

  .loading {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 1rem;
  }

  .presets-grid {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(250px, 1fr));
    gap: 1rem;
    margin-bottom: 1.5rem;
  }

  .preset-card {
    padding: 1.25rem;
    border: 2px solid var(--bg-lighter);
    border-radius: 12px;
    cursor: pointer;
    transition: all 0.2s;
    background-color: var(--bg-primary);
  }

  .preset-card:hover {
    border-color: var(--main);
    transform: translateY(-2px);
  }

  .preset-card.selected {
    border-color: var(--main);
    background-color: var(--bg-lighter);
  }

  .preset-card h3 {
    margin: 0 0 0.5rem 0;
    font-size: 1.125rem;
    font-weight: 600;
    color: var(--text-color);
  }

  .preset-card .description {
    margin: 0;
    font-size: 0.875rem;
    color: var(--text-color);
    opacity: 0.8;
    line-height: 1.4;
  }

  .actions {
    display: flex;
    justify-content: center;
    padding-top: 1rem;
    border-top: 2px solid var(--bg-lighter);
  }
</style>
