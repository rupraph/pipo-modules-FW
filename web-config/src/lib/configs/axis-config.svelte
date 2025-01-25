<script lang="ts" generics="T extends PipoTypes">
  import { pipoio } from "../../pipoio";
  import {
    isContinuousMode,
    isHisteresisMode,
    type AxisSchema,
    type SensorConfig,
    type SensorValues,
    type SmoothSensorValues,
    type PipoKeys,
  } from "../../types";
  import Checkbox from "../form/Checkbox.svelte";
  import Switch from "../form/Switch.svelte";
  import MinMax from "../form/MinMax.svelte";
  import axios from "axios";

  export let sensor: SensorConfig;
  export let aschema: AxisSchema;
  export let currentAxis: string;
  const smoothValues: SmoothSensorValues<T> = {};
  const withinWindowValues: SensorValues<T> = {};
  const sensorValues: SensorValues<T> = {};

  pipoio.on("sensor", ({ axis, value, withinWindow }) => {
    withinWindowValues[axis] = withinWindow;
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
  });

  function animateSensor() {
    Object.entries(smoothValues).forEach(([axis, value]) => {
      if (value.dt > 0) {
        sensorValues[axis] =
          value.old + (value.new - value.old) * (value.dt / 1000);
      }
    });
    requestAnimationFrame(animateSensor);
  }
  animateSensor();

  function cal_offset(axis: PipoKeys[T]) {
    axios({
      method: "post",
      url: "/offsetcal",
      params: { axis },
    }).then(() => console.log("DONE"));
  }
</script>

<div
  style="display: flex; align-items: left; justify-content: space-around; margin-bottom:1.5em"
>
  <Switch label="Inverted" bind:value={sensor.invert} design="slider" />
  {#if aschema.cat !== "Touch"}
    <Switch label="Threshold mode" bind:value={sensor.mode} design="slider" />
    <!-- {#if sensor.mode === true} -->
    <div class:disabled={!sensor.mode}>
      <Switch
        label="2-level threshold"
        bind:value={sensor.th_mode}
        design="slider"
      />
    </div>
    <!-- {/if} -->
  {/if}
</div>

<MinMax
  label="Sensor Range"
  bind:low={sensor.lmin}
  bind:high={sensor.lmax}
  value={sensorValues[currentAxis]}
  mode={isContinuousMode(sensor) || isHisteresisMode(sensor)
    ? "double"
    : "single"}
  cursorActive={withinWindowValues[currentAxis]}
  min={aschema.min}
  max={aschema.max}
  step={aschema.step}
  minLabel={`min (${aschema.unit})`}
  maxLabel={`max (${aschema.unit})`}
/>

{#if aschema.cat === "Touch"}
  <button class="primary" on:click={() => cal_offset(currentAxis)}
    >Zero offset calibration</button
  >
{/if}

<style>
  :global(.disabled) {
    opacity: 0.2;
    pointer-events: none;
  }
</style>
