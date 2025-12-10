<script lang="ts">
  import type { MidiConfig, InputSettings } from "../../types";
  import { currentConfig, currentMode, pipoType } from "../../services/config";
  import { uiState } from "../ui-state/store";
  import NoteConfig from "./note-config.svelte";
  import Number from "../form/Number.svelte";
  import InfoModal from "../InfoModal.svelte";
  import { TriangleAlert } from "lucide-svelte";

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

  $: isChannelEnabled = midiConfig?.enabled ?? false;

  // Track specific values for reactivity - these will update when the underlying values change
  $: currentCCNumber = midiConfig?.cc_nb;
  $: currentMidiChannel = midiConfig?.channel;
  $: currentTlMode = midiConfig?.tl_mode;
  $: currentRootNote = midiConfig?.rootNote;
  $: currentEnabled = midiConfig?.enabled;
  $: currentInputMode = input?.mode;

  // Force reactivity by creating a composite key that changes when any relevant value changes
  $: ccReactivityKey = `${currentMidiChannel}-${currentCCNumber}-${currentTlMode}`;
  $: noteReactivityKey = `${currentMidiChannel}-${currentRootNote}-${currentTlMode}`;
  $: midiChannelReactivityKey = `${currentMidiChannel}-${currentTlMode}-${currentInputMode}`;

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

  // Check if MIDI channel is occupied by another channel in Note+Continuous mode
  $: midiChannelConflictChannels = (() => {
    if (!config || !midiConfig || !selectedChannel || !input) return [];

    // Use tracked values and reactivity key to ensure reactivity
    const checkChannel = currentMidiChannel;
    const checkTlMode = currentTlMode;
    const checkInputMode = input.mode; // false = continuous, true = binary/threshold
    // Reference midiChannelReactivityKey to ensure this recalculates when values change
    const _ = midiChannelReactivityKey;

    // Only check for conflicts if current channel is in Note mode + Continuous mode
    if (checkTlMode !== 1 || checkInputMode !== false) return [];

    // Check all other channels to see if any are occupying this MIDI channel
    return Object.keys(config.engine["engine-midi"]).filter((key) => {
      if (key === selectedChannel) return false; // Skip current channel
      const otherMidiConfig = config.engine["engine-midi"][
        key as keyof (typeof config.engine)["engine-midi"]
      ] as MidiConfig;
      const otherInput = config.inputs[
        key as keyof typeof config.inputs
      ] as InputSettings;

      // Other channel occupies the MIDI channel if it's in Note mode + Continuous mode
      const otherOccupiesChannel =
        otherMidiConfig.tl_mode === 1 && otherInput.mode === false;

      // Conflict exists if another channel occupies this MIDI channel
      return otherMidiConfig.channel === checkChannel && otherOccupiesChannel;
    });
  })();

  $: midiChannelConflict = midiChannelConflictChannels.length > 0;

  // Check if current CC number conflicts with other channels
  $: ccConflictChannels = (() => {
    if (!config || !midiConfig || !selectedChannel) return [];
    if (currentTlMode !== 0) return []; // Only check in CC mode

    // Use tracked values and reactivity key to ensure reactivity
    const checkChannel = currentMidiChannel;
    const checkCC = currentCCNumber;
    // Reference ccReactivityKey to ensure this recalculates when values change
    const _ = ccReactivityKey;

    // Check all other channels and collect conflicting ones
    return Object.keys(config.engine["engine-midi"]).filter((key) => {
      if (key === selectedChannel) return false; // Skip current channel
      const otherMidiConfig = config.engine["engine-midi"][
        key as keyof (typeof config.engine)["engine-midi"]
      ] as MidiConfig;

      return (
        // Conflict if other channel is in CC mode (regardless of enabled state)
        otherMidiConfig.tl_mode === 0 && // Other is in CC mode
        otherMidiConfig.channel === checkChannel &&
        otherMidiConfig.cc_nb === checkCC
      );
    });
  })();

  $: ccConflict = ccConflictChannels.length > 0;

  // Check if current note conflicts with other channels
  $: noteConflictChannels = (() => {
    if (!config || !midiConfig || !selectedChannel) return [];
    if (currentTlMode !== 1) return []; // Only check in Note mode

    // Use tracked values and reactivity key to ensure reactivity
    const checkChannel = currentMidiChannel;
    const checkNote = currentRootNote;
    // Reference noteReactivityKey to ensure this recalculates when note changes
    const _ = noteReactivityKey;

    // Check all other channels and collect conflicting ones
    return Object.keys(config.engine["engine-midi"]).filter((key) => {
      if (key === selectedChannel) return false; // Skip current channel
      const otherMidiConfig = config.engine["engine-midi"][
        key as keyof (typeof config.engine)["engine-midi"]
      ] as MidiConfig;

      return (
        // Conflict if other channel is in Note mode (regardless of enabled state)
        otherMidiConfig.tl_mode === 1 && // Other is in Note mode
        otherMidiConfig.channel === checkChannel &&
        otherMidiConfig.rootNote === checkNote
      );
    });
  })();

  $: noteConflict = noteConflictChannels.length > 0;
</script>

{#if config && selectedChannel && midiConfig && input}
  <div class="output-settings">
    <div class="header-row">
      <h4>MIDI Output</h4>
      {#if !isChannelEnabled}
        <span class="warning-text">
          <TriangleAlert color="var(--red)" size={14} />
          Channel is disabled
        </span>
      {/if}
    </div>
    <!-- Message Type: Note/CC Pill Switch -->
    <div class="row">
      <span class="output-label">Message Type</span>
      <InfoModal>
        <p>
          You can choose to translate the sensor data into Midi Continous
          Controls, or to Midi Notes. If the sensor is put into "binary mode",
          this allows to trigger only one note. If not you can choose a note
          pattern and it will behave like a harp.
        </p>
      </InfoModal>
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
      <span class="output-label">Midi Channel</span>
      {#if midiChannelConflict}
        <span class="conflict-warning">
          <TriangleAlert size={14} color="var(--red)" />
          <span class="conflict-text"
            >"{midiChannelConflictChannels.join(", ")}" uses this channel too</span
          >
        </span>
      {:else}
        <span></span>
      {/if}
      <div class="input-container">
        <Number label="" bind:value={midiConfig.channel} min={1} max={16} />
      </div>
    </div>

    <!-- CC Mode Settings -->
    {#if midiConfig.tl_mode === 0}
      <div class="row">
        <span class="label">CC Number</span>
        {#if ccConflict}
          <span class="conflict-warning">
            <TriangleAlert size={14} color="var(--red)" />
            <span class="conflict-text"
              >CC also used in "{ccConflictChannels.join(", ")}"</span
            >
          </span>
        {:else}
          <span></span>
        {/if}
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
        <span class="output-label">CC Out Min</span>
        <span></span>
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
        <span class="output-label">CC Out Max</span>
        <span></span>
        <div class="input-container">
          <Number
            label=""
            bind:value={midiConfig.cc_max}
            min={midiConfig.cc_min}
            max={midiConfig.hires ? 16383 : 127}
          />
        </div>
      </div>

      <!-- <div class=\"row\">\n        <span class=\"output-label\">High Resolution</span>", "oldString": "      <!-- <div class=\"row\">\n        <span class=\"label\">High Resolution</span>"
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
      <NoteConfig
        config={midiConfig}
        bind:isThresholdMode={input.mode}
        hasConflict={noteConflict}
        conflictChannels={noteConflictChannels}
      />
    {/if}
  </div>
{/if}

<style>
  .output-settings {
    width: 100%;
  }

  .header-row {
    display: flex;
    align-items: center;
    justify-content: flex-start;
    gap: 16px;
    margin-bottom: 8px;
    min-height: 32px;
  }

  .header-row h4 {
    margin: 0;
    line-height: 1;
  }

  .warning-text {
    color: var(--red);
    font-size: 13px;
    font-weight: 600;
    display: flex;
    align-items: center;
    gap: 6px;
    line-height: 1;
    padding: 4px 8px;
    background-color: rgba(255, 0, 0, 0.1);
    border-radius: 4px;
  }

  .conflict-warning {
    color: var(--red);
    display: flex;
    align-items: center;
    justify-content: flex-start;
    gap: 4px;
    font-size: 12px;
    font-weight: 700;
    white-space: nowrap;
  }

  .conflict-text {
    font-size: 12px;
    font-weight: 700;
  }

  .row {
    display: grid;
    grid-template-columns: auto 24px 1fr;
    align-items: center;
    gap: 0.5em;
    margin-bottom: 4px;
  }

  .output-label {
    white-space: nowrap;
  }

  .input-container,
  .pill-switch {
    justify-self: end;
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

  /* Pill Switch - Component Specific */
  .pill-indicator.note {
    transform: translateX(calc(100% + 2px));
  }

  /* Balance label widths for CC (2 chars) and Note (4 chars) */
  .pill-switch label {
    min-width: 40px;
  }
</style>
