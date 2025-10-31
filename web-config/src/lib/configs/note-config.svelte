<script lang="ts">
  import { pipoType } from "../../services";
  import type { NoteConfig, BaseMidiConfig } from "../../types";
  import NoteInput from "../form/NoteInput.svelte";
  import Range from "../form/Range.svelte";
  import Select from "../form/Select.svelte";
  import Tooltip from "../tooltip/Tooltip.svelte";

  export let config: NoteConfig & BaseMidiConfig;
  export let isThresholdMode = false;

  const patternTypes = [
    { label: "Scale", value: "scale" },
    { label: "Arpeggio", value: "arpeggio" },
    { label: "Interval", value: "interval" },
  ];

  // this is currently defined both on pipo and config side.
  const scaleTypes = [
    { label: "Major", value: "major", notes: [0, 2, 4, 5, 7, 9, 11] },
    { label: "Minor", value: "minor", notes: [0, 2, 3, 5, 7, 8, 10] },
    {
      label: "Minor Pentatonic",
      value: "minor pentatonic",
      notes: [0, 3, 5, 6, 10],
    },
    {
      label: "Major Pentatonic",
      value: "major pentatonic",
      notes: [0, 2, 4, 7, 9],
    },
    { label: "Blues Minor", value: "blues minor", notes: [0, 3, 5, 6, 7, 10] },
    { label: "Blues Major", value: "blues major", notes: [0, 2, 3, 5, 6, 7] },
    {
      label: "Chromatic",
      value: "chromatic",
      notes: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11],
    },
    { label: "Whole Tone", value: "whole tone", notes: [0, 2, 4, 6, 8, 10] },
    {
      label: "Octatonic",
      value: "octatonic",
      notes: [0, 1, 3, 4, 6, 7, 9, 10],
    },
    { label: "Diatonic", value: "diatonic", notes: [0, 2, 4, 5, 7, 9, 11] },
    {
      label: "Harmonic minor",
      value: "harmonic minor",
      notes: [0, 2, 3, 5, 7, 8, 11],
    },
    {
      label: "Melodic minor",
      value: "melodic minor",
      notes: [0, 2, 3, 5, 7, 9, 11],
    },
    { label: "Dorian", value: "dorian", notes: [0, 2, 3, 5, 7, 9, 10] },
    { label: "Phrygian", value: "phrygian", notes: [0, 1, 3, 5, 7, 8, 10] },
    { label: "Lydian", value: "lydian", notes: [0, 2, 4, 6, 7, 9, 11] },
    { label: "Mixolydian", value: "mixolydian", notes: [0, 2, 4, 5, 7, 9, 10] },
    { label: "Locrian", value: "locrian", notes: [0, 1, 3, 5, 6, 8, 10] },
    { label: "Ionian", value: "ionian", notes: [0, 2, 4, 5, 7, 9, 11] },
    { label: "Aeolian", value: "aeolian", notes: [0, 2, 3, 5, 7, 8, 1] },
  ];

  const arpeggioTypes = [
    { label: "Major", value: "major", notes: [0, 4, 7] },
    { label: "Minor", value: "minor", notes: [0, 3, 7] },
    { label: "Diminished", value: "diminished", notes: [0, 3, 6] },
    { label: "Augmented", value: "augmented", notes: [0, 4, 8] },
    { label: "Suspended", value: "suspended", notes: [0, 5, 7] },
    { label: "Dominant", value: "dominant", notes: [0, 4, 7, 10] },
    { label: "Major 7", value: "major7", notes: [0, 4, 7, 11] },
    { label: "Minor 7", value: "minor7", notes: [0, 3, 7, 10] },
    { label: "Diminished 7", value: "diminished7", notes: [0, 3, 6, 9] },
    { label: "Augmented 7", value: "augmented7", notes: [0, 4, 8, 10] },
    { label: "Suspended 7", value: "suspended7", notes: [0, 5, 7, 10] },
    { label: "Dominant 7", value: "dominant7", notes: [0, 4, 7, 10] },
    { label: "Major 9", value: "major9", notes: [0, 4, 7, 11, 14] },
    { label: "Minor 9", value: "minor9", notes: [0, 3, 7, 10, 14] },
    { label: "Diminished 9", value: "diminished9", notes: [0, 3, 6, 9, 13] },
    { label: "Augmented 9", value: "augmented9", notes: [0, 4, 8, 10, 14] },
  ];

  const intervals = [
    { label: "Second", value: "second", notes: [0, 2] },
    { label: "Third", value: "third", notes: [0, 4] },
    { label: "Fourth", value: "fourth", notes: [0, 5] },
    { label: "Fifth", value: "fifth", notes: [0, 7] },
    { label: "Sixth", value: "sixth", notes: [0, 9] },
    { label: "Seventh", value: "seventh", notes: [0, 11] },
    { label: "Octave", value: "octave", notes: [0, 12] },
    { label: "Ninth", value: "ninth", notes: [0, 14] },
    { label: "Tenth", value: "tenth", notes: [0, 16] },
    { label: "Eleventh", value: "eleventh", notes: [0, 17] },
    { label: "Twelfth", value: "twelfth", notes: [0, 19] },
  ];
  $: {
    if (
      config.pattern === "scale" &&
      !scaleTypes.find((type) => type.value === config.scaleType)
    ) {
      config.scaleType = scaleTypes[0].value;
    } else if (
      config.pattern === "arpeggio" &&
      !arpeggioTypes.find((type) => type.value === config.scaleType)
    ) {
      config.scaleType = arpeggioTypes[0].value;
    } else if (
      config.pattern === "interval" &&
      !intervals.find((type) => type.value === config.scaleType)
    ) {
      config.scaleType = intervals[0].value;
    }
  }
</script>

<!-- {#if $pipoType !== "analog"} -->
<!-- <Tooltip title="The axis is in threshold mode" bind:enabled={isThresholdMode}> -->
<Select
  class={isThresholdMode ? "disabled" : ""}
  label="Pattern"
  options={patternTypes}
  bind:value={config.pattern}
/>
{#if config.pattern === "scale"}
  <Select
    class={isThresholdMode ? "disabled" : ""}
    label="Scale type"
    options={scaleTypes}
    bind:value={config.scaleType}
  />
{:else if config.pattern === "arpeggio"}
  <Select
    class={isThresholdMode ? "disabled" : ""}
    label="Arpeggio type"
    options={arpeggioTypes}
    bind:value={config.scaleType}
  />
{:else if config.pattern === "interval"}
  <Select
    class={isThresholdMode ? "disabled" : ""}
    label="Interval type"
    options={intervals}
    bind:value={config.scaleType}
  />
{/if}
<!-- </Tooltip> -->
<!-- {/if} -->

<NoteInput label="Root Note" bind:value={config.rootNote} />

<!-- {#if $pipoType !== "analog"} -->
<!-- <Tooltip title="The axis is in threshold mode" enabled={isThresholdMode}> -->
<Range
  class={isThresholdMode ? "disabled" : ""}
  label="Number of Notes"
  tooltip="You are in threshold mode, this value is ignored."
  bind:value={config.nbOfNotes}
  min={1}
  max={50}
/>
<!-- </Tooltip> -->
<!-- {/if} -->
<Range label="Sustain" bind:value={config.sustain} min={0} max={5} />
<Range label="Velocity" bind:value={config.velocity} min={0} max={127} />

<style>
</style>
