<script lang="ts">
  import { type SensorSettings } from "../../types";
  import { pipoType, pipoType as type } from "../../services";
  import Switch from "../form/Switch.svelte";
  import Collapse from "../Collapse.svelte";
  import Tooltip from "../Tooltip.svelte";
  export let config: SensorSettings;
</script>

{#if $type === "motion" || $type === "range"}
  <Collapse title="Sensor settings">
    {#if $type === "motion"}
      <Switch
        label="Relative (on) or absolute orientation"
        bind:value={config.relative_mode}
        design="slider"
      />
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
