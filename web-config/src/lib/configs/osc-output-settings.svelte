<script lang="ts">
  import type { OscConfig } from "../../types";
  import { currentConfig, pipoType } from "../../services/config";
  import { uiState } from "../ui-state/store";

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

  function toggleRawMode() {
    if (!oscConfig) return;
    oscConfig.mode_raw = !oscConfig.mode_raw;
    currentConfig.set(config);
  }
</script>

{#if config && selectedChannel && oscConfig}
  <div class="output-settings">
    <h4>OSC Output</h4>

    <!-- Enable/Disable Toggle -->
    <div class="row">
      <span class="label">Output State</span>
      <button class:enabled={oscConfig.enabled} on:click={toggleEnabled}>
        {oscConfig.enabled ? "Enabled" : "Disabled"}
      </button>
    </div>

    <!-- OSC Address -->
    <div class="row">
      <span class="label">OSC Address</span>
      <input
        type="text"
        bind:value={oscConfig.osc_addr}
        maxlength="255"
        class="text-input"
        placeholder="/address"
      />
    </div>

    <!-- Raw Mode Toggle -->
    <div class="row">
      <span class="label">Raw Mode</span>
      <button class:enabled={oscConfig.mode_raw} on:click={toggleRawMode}>
        {oscConfig.mode_raw ? "On" : "Off"}
      </button>
    </div>

    <!-- Min/Max Values (only when not in raw mode) -->
    {#if !oscConfig.mode_raw}
      <div class="row">
        <span class="label">Output Min</span>
        <input
          type="number"
          bind:value={oscConfig.osc_min}
          min="0"
          max={oscConfig.osc_max}
          class="number-input"
        />
      </div>

      <div class="row">
        <span class="label">Output Max</span>
        <input
          type="number"
          bind:value={oscConfig.osc_max}
          min={oscConfig.osc_min}
          class="number-input"
        />
      </div>
    {/if}
  </div>
{/if}

<style>
  .output-settings {
    width: 100%;
    background-color: var(--bg-secondary);
    margin-top: 24px;
  }

  h4 {
    font-size: 16px;
    font-weight: 700;
    margin-bottom: 16px;
    color: var(--main);
  }

  .row {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 16px;
  }

  .label {
    font-size: 14px;
    font-weight: 500;
    color: var(--main);
  }

  /* Enable/Disable Button */
  button {
    border: 2px solid var(--main);
    height: 29px;
    padding: 0 16px;
    background-color: var(--bg-secondary);
    border-radius: 18px;
    font-size: 12px;
    line-height: 16px;
    font-weight: 700;
    color: var(--main);
    cursor: pointer;
    transition: all 0.2s ease;
  }

  button.enabled {
    background-color: var(--main);
    color: var(--bg-primary);
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

  /* Number Input */
  .number-input {
    width: 80px;
    height: 32px;
    padding: 0 12px;
    border: 2px solid var(--main);
    border-radius: 6px;
    background-color: var(--bg-secondary);
    color: var(--main);
    font-size: 14px;
    font-weight: 500;
    text-align: center;
  }

  .number-input:focus {
    outline: none;
    border-color: var(--accent);
  }

  /* Remove spinner buttons */
  .number-input::-webkit-inner-spin-button,
  .number-input::-webkit-outer-spin-button {
    -webkit-appearance: none;
    margin: 0;
  }

  .number-input[type="number"] {
    -moz-appearance: textfield;
    appearance: textfield;
  }
</style>
