<script lang="ts">
  import type { NoteConfig, BaseMidiConfig } from "../../types";
  import Range from "../form/Range.svelte";
  import Select from "../form/Select.svelte";
  export let config: NoteConfig & BaseMidiConfig;

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
</script>

<Select label="Scale type" options={scaleTypes} bind:value={config.scaleType} />
<Range label="Root Note" bind:value={config.rootNote} min={0} max={127} />
<Range
  label="Number of Notes"
  bind:value={config.numberOfNotes}
  min={1}
  max={50}
/>

<style>
</style>
