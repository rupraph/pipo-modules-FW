<script lang="ts" generics="T extends PipoTypes">
  import { pipoio, PipoIO } from "../../pipoio";
  import {
    isContinuousMode,
    isHisteresisMode,
    type AxisSchema,
    type SensorConfig,
    type SensorValues,
    type SmoothSensorValues,
    type PipoKeys,
    type PipoTypes,
    type SmoothSensorValue,
  } from "../../types";
  import Switch from "../form/Switch.svelte";
  import MinMax from "../form/MinMax.svelte";

  export let sensor: SensorConfig;
  export let aschema: AxisSchema;
  export let currentAxis: PipoKeys[T];
  const smoothValues: Partial<SmoothSensorValues<T>> = {};
  const withinWindowValues: Partial<SensorValues<T>> = {};
  const sensorValues: Partial<SensorValues<T>> = {};

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

  function cal_offset(axis: PipoKeys[T]) {
    pipoio
      .request({
        method: "post",
        url: "/offsetcal",
        params: { axis },
      })
      .then(() => console.log("DONE"));
  }
</script>

<div
  style="display: flex; align-items: left; justify-content: space-around; margin-bottom:1.5em"
>
  <Switch label="Inverted" bind:value={sensor.inverted} design="slider" />
  {#if aschema.cat !== "Touch"}
    <Switch label="Cyclic" bind:value={sensor.cyclic} design="slider" />
    <Switch label="Threshold mode" bind:value={sensor.mode} design="slider" />
    <div class:disabled={!sensor.mode}>
      <Switch
        label="2-level threshold"
        bind:value={sensor.th_mode}
        design="slider"
      />
    </div>
  {/if}
</div>

<MinMax
  bind:low={sensor.lmin}
  bind:high={sensor.lmax}
  value={sensorValues[currentAxis]}
  mode={isContinuousMode(sensor) || isHisteresisMode(sensor)
    ? "double"
    : "single"}
  cursorActive={Boolean(withinWindowValues[currentAxis])}
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
