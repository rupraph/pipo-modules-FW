<script lang="ts">
  import { onMount } from "svelte";
  import Select from "svelte-select";
  import Input from "./Input.svelte";
  import { uid } from "../../utils";
  export let label: string;
  export let value: string;
  export let options: { value: string; label: string }[] = [];
  let selectedItem: { value: string; label: string };
  let id = uid();
  onMount(async () => {
    if (!value || !options.length) return;
    const toSelect = options.find((option) => option.value === value);
    if (!toSelect) return;
    selectedItem = toSelect;
  });
</script>

<Input {label} {id}>
  <Select
    items={options}
    bind:value={selectedItem}
    --selected-item-color="var(--text-color)"
    --item-is-active-bg="var(--bg-tertiary)"
    --item-color="var(--text-color)"
    --item-bg="var(--bg-secondary)"
    --input-color="var(--text-color)"
    --item-hover-color="var(--text-color)"
    --item-hover-bg="var(--bg-lighter)"
    --border-radius="0"
    --border="0"
    --border-focused="0"
    --list-background="var(--bg-secondary)"
    --background="var(--bg-secondary)"
    on:change={(e) => (value = e.detail.value)}
  />
</Input>

<style>
</style>
