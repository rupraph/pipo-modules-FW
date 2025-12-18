<script lang="ts">
  import { onMount } from "svelte";
  import { uid } from "../../utils";
  import Input from "./Input.svelte";
  import Range from "./Range.svelte";
  import Select from "./Select.svelte";
  export let value: number;
  export let label: string = "";
  import Number from "./Number.svelte";

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

  const octaveOptions = Array.from({ length: 10 }, (_, i) => ({
    label: i.toString(),
    value: i,
  }));

  let id = uid();
  function onChange() {
    if (isNaN(value)) return;
    note = nOptions[value % 12].value;
    octave = Math.floor(value / 12) - 1;
  }
  onMount(() => {
    onChange();
  });

  $: {
    if (!isNaN(note) && !isNaN(octave)) {
      value = note + (octave + 1) * 12;
    }
  }
  $: {
    if (!isNaN(value)) {
      onChange();
    }
  }
</script>

<!-- <Input {label} {id}> -->
<div class="noteinput">
  {#if typeof note === "number" && typeof octave === "number"}
    <Select
      class="autocolumns"
      label="Note"
      --width="70px"
      options={nOptions}
      bind:value={note}
    />
    <Select
      class="autocolumns"
      label="Octave"
      --width="70px"
      options={octaveOptions}
      bind:value={octave}
    />
  {/if}
</div>

<!-- </Input> -->

<style>
  .noteinput {
    display: flex;
    width: 100%;
  }
  /* .noteinput > :global(.input) {
    grid-template-columns: 50px auto;
  } */

  :global(.svelte-select-list::-webkit-scrollbar) {
    width: 8px;
    background: var(--bg-secondary);
  }

  :global(.svelte-select-list::-webkit-scrollbar-thumb) {
    background: var(--bg-tertiary);
    border-radius: 4px;
  }
</style>
