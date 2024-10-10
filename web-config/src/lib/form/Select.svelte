<script lang="ts">
  import { onMount } from "svelte";
  import Select from "svelte-select";
  import Input from "./Input.svelte";
  import { uid } from "../../utils";
  export let label: string;
  export let clearable: boolean = false;
  export let value: string | number;
  export let options: { value: string | number; label: string }[] = [];
  let selectedItem: { value: string | number; label: string };
  let id = uid();
  function select() {
    if (!value || !options.length) return;
    const toSelect = options.find((option) => option.value === value);
    if (!toSelect) return;
    selectedItem = toSelect;
  }
  onMount(() => {
    select();
  });
  $: {
    if (value !== undefined) {
      select();
    }
  }
</script>

<Input {label} {id} class={$$restProps.class || ""}>
  <Select
    items={options}
    {clearable}
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
