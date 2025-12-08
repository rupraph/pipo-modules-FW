<script lang="ts">
  import type { OscConfig } from "../../types";
  import { currentConfig, pipoType } from "../../services/config";
  import { uiState } from "../ui-state/store";
  import Number from "../form/Number.svelte";

  $: config = $currentConfig;
  $: type = $pipoType;
  $: selectedChannel = $uiState[type]?.selectedChannel;

  $: oscConfig =
    config && selectedChannel
      ? (config.engine["engine-osc"][
          selectedChannel as keyof (typeof config.engine)["engine-osc"]
        ] as OscConfig)
      : null;

  function toggleEnabled() {
    if (!oscConfig) return;
    oscConfig.enabled = !oscConfig.enabled;
    currentConfig.set(config);
  }

  function validateOscAddress(event: Event) {
    const input = event.target as HTMLInputElement;
    // Remove spaces and special characters, keep only alphanumeric, slash, hyphen, underscore, and dot
    const cleaned = input.value.replace(/[^a-zA-Z0-9/_\-\.]/g, "");
    if (cleaned !== input.value) {
      input.value = cleaned;
      if (oscConfig) {
        oscConfig.osc_addr = cleaned;
        currentConfig.set(config);
      }
    }
  }

  function handleOscAddressKeydown(event: KeyboardEvent) {
    // Prevent space and other invalid characters from being entered
    if (event.key === " " || /[^a-zA-Z0-9/_\-\.]/.test(event.key)) {
      // Allow navigation and control keys
      if (
        !event.ctrlKey &&
        !event.metaKey &&
        event.key !== "Backspace" &&
        event.key !== "Delete" &&
        event.key !== "ArrowLeft" &&
        event.key !== "ArrowRight" &&
        event.key !== "Tab"
      ) {
        event.preventDefault();
      }
    }
  }
</script>

{#if config && selectedChannel && oscConfig}
  <div class="output-settings">
    <h4>OSC Output</h4>
    <!-- OSC Address -->
    <div class="row">
      <span class="label">OSC Address</span>
      <div class="input-container">
        <input
          type="text"
          bind:value={oscConfig.osc_addr}
          maxlength="255"
          class="text-input"
          placeholder="/address"
          on:input={validateOscAddress}
          on:keydown={handleOscAddressKeydown}
        />
      </div>
    </div>

    <!-- Min/Max Values (only when not in raw mode) -->
    {#if !oscConfig.mode_raw}
      <div class="row">
        <span class="label">Output Min</span>
        <div class="input-container">
          <Number
            label=""
            bind:value={oscConfig.osc_min}
            min={0}
            max={oscConfig.osc_max}
          />
        </div>
      </div>

      <div class="row">
        <span class="label">Output Max</span>
        <div class="input-container">
          <Number
            label=""
            bind:value={oscConfig.osc_max}
            min={oscConfig.osc_min}
          />
        </div>
      </div>
    {/if}
  </div>
{/if}

<style>
  .output-settings {
    width: 100%;
  }

  h4 {
    font-size: 16px;
    font-weight: 700;
    margin-bottom: 4px;
    color: var(--main);
  }

  .row {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 4px;
  }

  .label {
    font-size: 14px;
    font-weight: 500;
    color: var(--main);
  }

  /* Input Container */
  .input-container {
  }

  .input-container :global(.input) {
    display: flex;
    flex-direction: row;
    align-items: center;
    gap: 8px;
  }

  .input-container :global(.input label) {
    display: none;
  }

  .input-container :global(.input-wrapper) {
    width: auto;
    min-width: 80px;
  }

  /* Text Input */
  .text-input {
    width: 200px;
    height: 32px;
    padding: 0 12px;
    border: 2px solid var(--main);
    border-radius: 6px;
    background-color: var(--bg-secondary);
    color: var(--main);
    font-size: 14px;
    font-weight: 500;
  }

  .text-input:focus {
    outline: none;
    border-color: var(--accent);
  }

  .text-input::placeholder {
    color: var(--grey);
    opacity: 0.5;
  }
</style>
