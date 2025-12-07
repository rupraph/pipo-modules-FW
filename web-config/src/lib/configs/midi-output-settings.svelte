<script lang="ts">
  import type { MidiConfig, InputSettings } from "../../types";
  import { currentConfig, currentMode, pipoType } from "../../services/config";
  import { uiState } from "../ui-state/store";
  import NoteConfig from "./note-config.svelte";
  import Number from "../form/Number.svelte";

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
      <div class="input-container">
        <Number label="" bind:value={midiConfig.channel} min={1} max={16} />
      </div>
    </div>

    <!-- CC Mode Settings -->
    {#if midiConfig.tl_mode === 0}
      <div class="row">
        <span class="label">CC Number</span>
        <div class="input-container">
          <Number
            label=""
            bind:value={midiConfig.cc_nb}
            min={0}
            max={127}
            step={1}
          />
        </div>
      </div>

      <div class="row">
        <span class="label">CC Out Min</span>
        <div class="input-container">
          <Number
            label=""
            bind:value={midiConfig.cc_min}
            min={0}
            max={midiConfig.cc_max}
          />
        </div>
      </div>

      <div class="row">
        <span class="label">CC Out Max</span>
        <div class="input-container">
          <Number
            label=""
            bind:value={midiConfig.cc_max}
            min={midiConfig.cc_min}
            max={midiConfig.hires ? 16383 : 127}
          />
        </div>
      </div>

      <!-- <div class="row">
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
      </div> -->
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

  /* Input Container */
  .input-container {
    display: flex;
    justify-content: flex-end;
    align-items: center;
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

  /* Pill Switch Container */
  .pill-switch {
  }

  /* Pill Switch - Component Specific */
  .pill-indicator.note {
    transform: translateX(calc(100% + 2px));
  }
</style>
