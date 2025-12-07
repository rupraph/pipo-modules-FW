<script lang="ts">
  import type { MidiConfig, InputSettings } from "../../types";
  import { currentConfig, currentMode, pipoType } from "../../services/config";
  import { uiState } from "../ui-state/store";
  import NoteConfig from "./note-config.svelte";

  $: config = $currentConfig;
  $: type = $pipoType;
  $: selectedChannel = $uiState[type]?.selectedChannel;

  $: input =
    config && selectedChannel
      ? (config.inputs[
          selectedChannel as keyof typeof config.inputs
        ] as InputSettings)
      : null;

  $: midiConfig =
    config && selectedChannel
      ? (config.engine["engine-midi"][
          selectedChannel as keyof (typeof config.engine)["engine-midi"]
        ] as MidiConfig)
      : null;

  function toggleEnabled() {
    if (!midiConfig) return;
    midiConfig.enabled = !midiConfig.enabled;
    currentConfig.set(config);
  }

  function setMessageType(type: "note" | "cc") {
    if (!midiConfig) return;
    midiConfig.tl_mode = type === "note" ? 1 : 0;
    currentConfig.set(config);
  }

  // Prepare structure for future MIDI channel conflict validation
  // function getUsedMidiChannels(): Set<number> {
  //   if (!config) return new Set();
  //   const used = new Set<number>();
  //   Object.keys(config.engine["engine-midi"]).forEach(key => {
  //     if (key !== selectedChannel && config.engine["engine-midi"][key].enabled) {
  //       used.add(config.engine["engine-midi"][key].channel);
  //     }
  //   });
  //   return used;
  // }
</script>

{#if config && selectedChannel && midiConfig && input}
  <div class="output-settings">
    <h4>MIDI Output</h4>

    <!-- Enable/Disable Toggle -->
    <div class="row">
      <span class="label">Output State</span>
      <button class:enabled={midiConfig.enabled} on:click={toggleEnabled}>
        {midiConfig.enabled ? "Enabled" : "Disabled"}
      </button>
    </div>

    <!-- Message Type: Note/CC Pill Switch -->
    <div class="row">
      <span class="label">Message Type</span>
      <div class="pill-switch">
        <div class="pill-indicator" class:note={midiConfig.tl_mode === 1}></div>
        <input
          type="radio"
          name="message-type-{selectedChannel}"
          value="cc"
          id="cc-{selectedChannel}"
          checked={midiConfig.tl_mode === 0}
          on:change={() => setMessageType("cc")}
        />
        <input
          type="radio"
          name="message-type-{selectedChannel}"
          value="note"
          id="note-{selectedChannel}"
          checked={midiConfig.tl_mode === 1}
          on:change={() => setMessageType("note")}
        />
        <label
          for="cc-{selectedChannel}"
          on:click={() => setMessageType("cc")}
          class:active={midiConfig.tl_mode === 0}
        >
          CC
        </label>
        <label
          for="note-{selectedChannel}"
          on:click={() => setMessageType("note")}
          class:active={midiConfig.tl_mode === 1}
        >
          Note
        </label>
      </div>
    </div>

    <!-- MIDI Channel -->
    <div class="row">
      <span class="label">MIDI Channel</span>
      <input
        type="number"
        bind:value={midiConfig.channel}
        min="1"
        max="16"
        class="number-input"
      />
    </div>

    <!-- CC Mode Settings -->
    {#if midiConfig.tl_mode === 0}
      <div class="row">
        <span class="label">CC Number</span>
        <input
          type="number"
          bind:value={midiConfig.cc_nb}
          min="0"
          max="127"
          step="1"
          class="number-input"
        />
      </div>

      <div class="row">
        <span class="label">CC Out Min</span>
        <input
          type="number"
          bind:value={midiConfig.cc_min}
          min="0"
          max={midiConfig.cc_max}
          class="number-input"
        />
      </div>

      <div class="row">
        <span class="label">CC Out Max</span>
        <input
          type="number"
          bind:value={midiConfig.cc_max}
          min={midiConfig.cc_min}
          max={midiConfig.hires ? 16383 : 127}
          class="number-input"
        />
      </div>

      <div class="row">
        <span class="label">High Resolution</span>
        <button
          class:enabled={midiConfig.hires}
          on:click={() => {
            midiConfig.hires = !midiConfig.hires;
            currentConfig.set(config);
          }}
        >
          {midiConfig.hires ? "On" : "Off"}
        </button>
      </div>
    {/if}

    <!-- Note Mode Settings -->
    {#if midiConfig.tl_mode === 1}
      <NoteConfig config={midiConfig} bind:isThresholdMode={input.mode} />
    {/if}
  </div>
{/if}

<style>
  .output-settings {
    width: 100%;
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

  /* Pill Switch Styling */
  .pill-switch {
    border: 2px solid var(--main);
    position: relative;
    display: inline-flex;
    background-color: var(--bg-primary);
    border-radius: 20px;
    padding: 2px;
    gap: 2px;
    height: 36px;
  }

  .pill-indicator {
    position: absolute;
    top: 2px;
    left: 2px;
    width: calc(50% - 2px);
    height: calc(100% - 4px);
    background-color: var(--main);
    border-radius: 18px;
    transition: transform 0.3s ease;
    z-index: 0;
  }

  .pill-indicator.note {
    transform: translateX(100%);
  }

  .pill-switch input[type="radio"] {
    display: none;
  }

  .pill-switch label {
    position: relative;
    z-index: 1;
    padding: 6px 20px;
    cursor: pointer;
    font-size: 14px;
    font-weight: 700;
    color: var(--main);
    transition: color 0.3s ease;
    user-select: none;
    display: flex;
    align-items: center;
  }

  .pill-switch label.active {
    color: var(--bg-primary);
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
