<script lang="ts">
  import Select from "../form/Select.svelte";
  export let mode: number;

  const modes = [
    // { label: "Gamepad", value: 0 },
    { label: "Mouse", value: 1 },
    { label: "Keyboard", value: 2 },
  ];
</script>

<Select label="Mode" options={modes} bind:value={mode} />
<p>Please reboot after changing mode</p>

<style>
  /* .global-config {
      background-color: var(--bg-tertiary);
      margin-top: 1em;
    } */
</style>
