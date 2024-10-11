<script lang="ts" generics="T extends PipoTypes">
  import { pipoio } from "../../pipoio";
  import {
    isContinuousMode,
    isHisteresisMode,
    type AxisSchema,
    type SensorConfig,
    type SensorValues,
    type SmoothSensorValues,
  } from "../../types";
  import Checkbox from "../form/Checkbox.svelte";
  import MinMax from "../form/MinMax.svelte";

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
</script>

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
{#if aschema.cat !== "Touch"}
  <Checkbox label="Threshold mode" bind:value={sensor.mode} />
  {#if sensor.mode === true}
    <Checkbox label="Window threshold" bind:value={sensor.th_mode} />
  {/if}
{/if}
