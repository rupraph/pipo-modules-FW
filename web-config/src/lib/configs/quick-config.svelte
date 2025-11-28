<script lang="ts" generics="T extends PipoTypes">
  import type { Schema, PipoConfig, PipoKeys, PipoTypes } from "../../types";
  import { pipoType as type } from "../../services";
  import { schema } from "../../schema";
  export let config: PipoConfig<T>;
  let columns: number = 0;
  let headers: PipoKeys[T][] = [];

  // Determine current mode based on config
  $: currentMode = config?.general?.MidiEnabled ? "MIDI" : "OSC";
  $: currentEngineKey = config?.general?.MidiEnabled
    ? "engine-midi"
    : "engine-osc";

  $: {
    if (config) {
      columns = Object.keys(schema[$type]).length + 1;
      headers = (Object.entries(schema[$type]) as Array<[PipoKeys[T], any]>)
        .sort((a, b) => a[1].index - b[1].index)
        .map(([key]) => key);
    }
  }
</script>

<div class="wrapper scroll">
  <div class="quick-settings" style="--columns: {columns}">
    <span class="row-header" style="grid-row=1">Axis</span>
    {#each headers as header, i}
      <div class="header" style="grid-column:{i + 2}">{header}</div>
    {/each}
    <span class="row-header" style="grid-row=2">{currentMode}</span>
    {#each headers as header, j}
      <label class="checkbox" style="grid-row:2; grid-column:{j + 2}">
        <input
          type="checkbox"
          class="checkbox__input"
          checked={currentEngineKey === "engine-midi"
            ? config.engine["engine-midi"][header].enabled
            : config.engine["engine-osc"][header].enabled}
          on:change={(e) => {
            const checked = e.currentTarget.checked;
            if (currentEngineKey === "engine-midi") {
              config.engine["engine-midi"][header].enabled = checked;
            } else {
              config.engine["engine-osc"][header].enabled = checked;
            }
          }}
        />
      </label>
    {/each}
  </div>
</div>

<style>
  .wrapper {
    max-width: 100%;
    overflow-x: auto;
  }
  .quick-settings {
    display: grid;
    justify-items: center;
    grid-template-columns: repeat(var(--columns), 1fr);
    grid-template-rows: repeat(2, auto);
    grid-gap: 0.5em;
    padding-bottom: 1em;
    padding-right: 1em;
    width: max-content;
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
