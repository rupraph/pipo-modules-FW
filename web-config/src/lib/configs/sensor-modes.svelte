<script lang="ts">
  import { type SensorSettings } from "../../types";
  import { pipoType as type } from "../../services";
  import Switch from "../form/Switch.svelte";
  import Collapse from "../collapse.svelte";
  import Tooltip from "../tooltip/Tooltip.svelte";
  import { pipoio } from "../../pipoio";
  export let config: SensorSettings;

  function setReference() {
    pipoio.get("/setreference").then(() => {
      console.log("Reference orientation reset");
    });
  }
</script>

{#if $type === "motion" || $type === "range"}
  <Collapse title="Sensor settings">
    {#if $type === "motion"}
      <Switch
        label="Relative (on) or absolute orientation"
        bind:value={config.relative_mode}
        design="slider"
      />
      <Tooltip title="This will set the 0 for relative orentation">
        <button class="primary" on:click={setReference}
          >Set relative reference</button
        >
      </Tooltip>
    {/if}
    {#if $type === "range"}
      <Tooltip title="Hold mode will hold the last value if nothing in range">
        <Switch
          label="Hold mode"
          bind:value={config.hold_mode}
          design="slider"
        />
      </Tooltip>
    {/if}
  </Collapse>
{/if}
