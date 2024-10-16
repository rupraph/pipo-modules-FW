<script lang="ts" generics="T extends PipoTypes">
  import type { Schema, PipoConfig, PipoKeys, PipoTypes } from "../../types";

  export let config: PipoConfig<T>;
  export let schema: Schema<T>;
  let columns: number = 0;
  let headers: PipoKeys[T][] = [];
  const rows: {
    key: string;
    value: keyof PipoConfig<T>["engine"];
  }[] = [
    {
      key: "MIDI",
      value: "engine-midi",
    },
    {
      key: "OSC",
      value: "engine-osc",
    },
    {
      key: "HID",
      value: "engine-hid",
    },
  ];
  $: {
    if (config) {
      columns = Object.keys(schema).length + 1;
      headers = Object.entries(schema)
        .sort((a, b) => a[1].index - b[1].index)
        .map(([key]) => key) as PipoKeys[T][];
    }
  }
</script>

<div class="quick-settings" style="--columns: {columns}">
  <span class="row-header" style="grid-row=1">Axis</span>
  {#each headers as header, i}
    <div class="header" style="grid-column:{i + 2}">{header}</div>
  {/each}
  {#each rows as row, i}
    <span class="row-header" style="grid-row={i + 2}">{row.key}</span>
    {#each headers as header, j}
      <label class="checkbox" style="grid-row:{i + 2}; grid-column:{j + 2}">
        <input
          type="checkbox"
          class="checkbox__input"
          bind:checked={config.engine[row.value][header].enabled}
        />
      </label>
    {/each}
  {/each}
</div>

<style>
  .quick-settings {
    display: grid;
    justify-items: center;
    grid-template-columns: repeat(var(--columns), 1fr);
    grid-template-rows: 4;
    grid-gap: 1em;
  }
  .header {
    grid-row: 1;
  }

  .row-header {
    grid-column: 1;
  }
  input[type="checkbox"] {
    width: 0;
    height: 0;
    visibility: hidden;
  }
  .checkbox {
    width: 24px;
    height: 24px;
    border-radius: 50%;
    border: 1px solid #626262;
    background: var(--bg-tertiary) no-repeat center;
  }
  .checkbox:has(input:checked) {
    border-color: var(--main);
    background-color: var(--main);
  }
</style>
