<script lang="ts" generics="T extends PipoTypes">
  import type { PipoConfig, PipoKeys, PipoTypes } from "../../types";

  export let config: PipoConfig<T>;
  let columns: number = 0;
  let headers: PipoKeys[T][] = [];
  $: {
    if (config) {
      columns = Object.keys(config.engine["engine-hid"]).length + 1;
      headers = Object.keys(config.engine["engine-hid"]);
    }
  }
</script>

<div class="quick-settings" style="--columns: {columns}">
  <span class="row-header" style="grid-row=1">Axis</span>
  <span class="row-header" style="grid-row=2">MIDI</span>
  <span class="row-header" style="grid-row=3">OSC</span>
  <span class="row-header" style="grid-row=4">HID</span>
  {#each headers as header, i}
    <div class="header" style="grid-column:{i + 2}">{header}</div>
  {/each}
  {#each headers as header, i}
    <input
      type="checkbox"
      style="grid-row:2; grid-column:{i + 2}"
      bind:checked={config.engine["engine-midi"][header].enabled}
    />
  {/each}
  {#each headers as header, i}
    <input
      type="checkbox"
      style="grid-row:3; grid-column:{i + 2}"
      bind:checked={config.engine["engine-osc"][header].enabled}
    />
  {/each}
  {#each headers as header, i}
    <input
      type="checkbox"
      style="grid-row:4; grid-column:{i + 2}"
      bind:checked={config.engine["engine-hid"][header].enabled}
    />
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
  input {
    width: 1em;
    height: 1em;
  }

  .row-header {
    grid-column: 1;
  }
</style>
