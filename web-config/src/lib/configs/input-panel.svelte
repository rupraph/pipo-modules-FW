<script lang="ts" generics="T extends PipoTypes">
  import Tooltip from "../tooltip/Tooltip.svelte";

  import { pipoio, PipoIO } from "../../pipoio";
  import {
    isContinuousMode,
    isHisteresisMode,
    type AxisSchema,
    type InputSettings,
    type SensorValues,
    type SmoothSensorValues,
    type PipoKeys,
    type PipoTypes,
    type SmoothSensorValue,
  } from "../../types";
  import Switch from "../form/Switch.svelte";
  import MinMax from "../form/MinMax.svelte";

  export let input: InputSettings;
  export let aschema: AxisSchema;
  export let currentAxis: PipoKeys[T];
  const smoothValues: Partial<SmoothSensorValues<T>> = {};
  const withinWindowValues: Partial<SensorValues<T>> = {};
  const sensorValues: Partial<SensorValues<T>> = {};
  let maxSensorValue = aschema.max; // Initialize with the default lmax value

  $: {
    maxSensorValue = aschema.max;
  }

  (pipoio as PipoIO<T>).on("sensor", ({ axis, value, withinWindow }) => {
    withinWindowValues[axis] = +withinWindow;
    const now = Date.now();
    if (!smoothValues[axis]) {
      smoothValues[axis] = {
        new: value,
        old: value,
        dt: 0,
        timestamp: now,
      };
      sensorValues[axis] = value;
    }
    const dt = now - smoothValues[axis].timestamp;
    smoothValues[axis].dt = dt;
    smoothValues[axis].timestamp = now;
    smoothValues[axis].old = smoothValues[axis].new;
    smoothValues[axis].new = value;
    // maxSensorValue = aschema.max;
  });

  function animateSensor() {
    (
      Object.entries(smoothValues) as unknown as Array<
        [PipoKeys[T], SmoothSensorValue]
      >
    ).forEach(([axis, value]) => {
      if (value.dt > 0) {
        sensorValues[axis] =
          value.old + (value.new - value.old) * (value.dt / 1000);
      }
    });
    requestAnimationFrame(animateSensor);
  }
  animateSensor();
  $: {
    const currentValue = sensorValues[currentAxis];
    if (
      currentValue !== undefined &&
      currentValue > maxSensorValue &&
      currentValue > aschema.max
    ) {
      maxSensorValue = Math.round(currentValue); // Update maxSensorValue if a higher value is reached
    }
  }

  // $: display_min = (aschema.min - input.offset).toFixed(2);
  // $: display_max = (maxSensorValue - input.offset).toFixed(2);
</script>

<MinMax
  bind:low={input.lmin}
  bind:high={input.lmax}
  value={sensorValues[currentAxis]}
  mode={isContinuousMode(input) || isHisteresisMode(input)
    ? "double"
    : "single"}
  cursorActive={Boolean(withinWindowValues[currentAxis])}
  min={aschema.min}
  bind:max={maxSensorValue}
  step={aschema.step}
  minLabel={`LowLim (${aschema.unit})`}
  maxLabel={`HighLim (${aschema.unit})`}
  units={aschema.unit}
/>

<!-- {#if aschema.cat === "Touch"  } -->

<div class="container">
  {#if aschema.cat !== "Touch"}
    <div class="item">
      <Tooltip title="On/Off output above/below level">
        <Switch label="Threshold" bind:value={input.mode} design="slider" />
      </Tooltip>
    </div>
    <div class="item" class:disabled={!input.mode}>
      <Switch
        label="2-level threshold"
        bind:value={input.th_mode}
        design="slider"
      />
    </div>
    <div class="item">
      <Tooltip title="Output will loop to min" enabled>
        <Switch label="Cyclic" bind:value={input.cyclic} design="slider" />
      </Tooltip>
    </div>
  {/if}
  <div class="item">
    <Tooltip title="Invert the sensor output" enabled={true}>
      <Switch label="Invert" bind:value={input.inverted} design="slider" />
    </Tooltip>
  </div>
  {#if aschema.cat === "dist"}
    <div class="item">
      <Tooltip title="no object retunrs max or 0" enabled={true}>
        <Switch label="Over-out" bind:value={input.over_out} design="slider" />
      </Tooltip>
    </div>
  {/if}
</div>

<!-- {/if} -->

<style>
  .container {
    display: flex;
    flex-wrap: wrap;
    justify-content: space-around; /* Adjust as needed: space-between, space-evenly, etc. */
    align-items: center;
  }
  .item {
    flex: 1 1 20%; /* Adjust the basis percentage to control item width */
    margin: 10px; /* Adjust margin as needed */
    text-align: center; /* Center text horizontally */
  }
</style>
