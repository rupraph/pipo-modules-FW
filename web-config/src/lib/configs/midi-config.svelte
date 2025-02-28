<script lang="ts">
  import CCConfig from "./cc-config.svelte";
  import NoteConfig from "./note-config.svelte";
  import type { MidiConfig } from "../../types";
  import Radio from "../form/Radio.svelte";
  import Range from "../form/Range.svelte";

  export let midi: MidiConfig;
  export let sensormode: boolean;
  const options = [
    { label: "Note", value: "1" },
    { label: "CC", value: "0" },
  ];

  $: console.log(midi.enabled);
</script>

<Range label="Midi Channel" bind:value={midi.channel} min={1} max={16} />

<Radio
  label="Message Type"
  {options}
  value={midi.tl_mode}
  on:change={(evt) => {
    midi.tl_mode = evt.detail;
  }}
/>

{#if midi.tl_mode === 0}
  <CCConfig config={midi} />
{:else}
  <NoteConfig config={midi} bind:isThresholdMode={sensormode} />
{/if}
