<script lang="ts">
  import { onMount } from "svelte";
  import { uid } from "../../utils";
  import Input from "./Input.svelte";
  import Range from "./Range.svelte";
  import Select from "./Select.svelte";
  export let value: number;
  export let label: string = "";

  let note: number;
  let octave: number;
  const nOptions = [
    "C",
    "C#",
    "D",
    "D#",
    "E",
    "F",
    "F#",
    "G",
    "G#",
    "A",
    "A#",
    "B",
  ].map((label, i) => ({ label, value: i }));

  const oOptions = new Array(11)
    .fill(0)
    .map((_, i) => ({ label: `${i}`, value: i }));

  let id = uid();
  onMount(() => {
    if (isNaN(value)) return;
    note = nOptions[value % 12].value;
    octave = Math.floor(value / 12);
    console.log("NoteInput", { note, octave, nOptions, oOptions });
  });

  $: {
    if (!isNaN(note) && !isNaN(octave)) value = note + octave * 12;
  }
</script>

<Input {label} {id}>
  <div class="noteinput">
    {#if note && octave}
      <Select
        class="autocolumns"
        label="Note"
        options={nOptions}
        bind:value={note}
      />
      <Range
        class="autocolumns"
        label="Octave"
        bind:value={octave}
        min={0}
        max={10}
      />
    {/if}
  </div>
</Input>

<style>
  .noteinput {
    width: 100%;
    padding: 1rem;
  }
  .noteinput > :global(.input) {
    grid-template-columns: 50px auto;
  }
</style>
