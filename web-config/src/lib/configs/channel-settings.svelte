<script lang="ts">
  import type {
    AnalogOut,
    AnalogOutKeys,
    PipoConfig,
    PipoKeys,
    SensorValues,
    SmoothSensorValues,
    SmoothSensorValue,
  } from "../../types";
  import { isContinuousMode, isHisteresisMode } from "../../types";
  type Keys = PipoKeys["analog"];
  import { currentConfig, currentMode, pipoType } from "../../services/config";
  import { uiState } from "../ui-state";
  import InfoModal from "../InfoModal.svelte";
  import MinMax from "../form/MinMax.svelte";
  import { schema } from "../../schema";
  import { pipoio, PipoIO } from "../../pipoio";

  let category: "analog" | "touch" = "analog";

  $: config = $currentConfig;
  $: type = $pipoType;
  $: mode = $currentMode;
  $: engineKey = (mode === "MIDI" ? "engine-midi" : "engine-osc") as
    | "engine-midi"
    | "engine-osc";
  $: keys =
    category === "analog"
      ? Array.from({ length: 8 }, (_, i) => `A0${i + 1}` as Keys)
      : Array.from({ length: 8 }, (_, i) => `T${i + 1}` as Keys);

  $: selectedChannel = $uiState[type]?.selectedChannel;
  $: input = config && selectedChannel ? config.inputs[selectedChannel] : null;
  $: aschema = selectedChannel ? schema[type][selectedChannel] : null;

  let sensorValue: number | undefined = undefined;
  let withinWindow = false;
  let maxSensorValue = 3.3;
  let smoothValue: SmoothSensorValue = {
    new: 0,
    old: 0,
    dt: 0,
    timestamp: Date.now(),
  };

  $: if (aschema) {
    maxSensorValue = aschema.max;
  }

  $: channelConfig =
    config && selectedChannel
      ? config.engine[engineKey][selectedChannel]
      : null;

  pipoio.on("sensor", ({ axis, value, withinWindow: ww }) => {
    if (axis !== selectedChannel) return;

    withinWindow = Boolean(ww);
    const now = Date.now();
    const dt = now - smoothValue.timestamp;
    smoothValue.dt = dt;
    smoothValue.timestamp = now;
    smoothValue.old = smoothValue.new;
    smoothValue.new = value;
  });

  function animateSensor() {
    if (smoothValue.dt > 0) {
      sensorValue =
        smoothValue.old +
        (smoothValue.new - smoothValue.old) * (smoothValue.dt / 1000);
    }
    requestAnimationFrame(animateSensor);
  }
  animateSensor();

  $: console.log("Channel Settings State:", {
    config: !!config,
    selectedChannel,
    channelConfig: !!channelConfig,
    type,
    uiState: $uiState,
  });

  $: if (
    sensorValue !== undefined &&
    aschema &&
    sensorValue > maxSensorValue &&
    sensorValue > aschema.max
  ) {
    maxSensorValue = Math.round(sensorValue);
  }
</script>

{#if config && selectedChannel && channelConfig}
  <div class="row">
    <span class="label">Channel state</span>
    <div class="buttons">
      <button
        class:enabled={!channelConfig.enabled}
        on:click={() => {
          channelConfig.enabled = !channelConfig.enabled;
          // Trigger store update to notify other components
          currentConfig.set(config);
        }}
      >
        Mute
      </button>
    </div>
  </div>
  <div class="row">
    <div class="left">
      <span class="label">Options</span>
      <InfoModal>Information about options</InfoModal>
    </div>
    <div class="buttons">
      <button class="rounder primary">Range invert </button>
      <button class="rounder primary">Binary mode </button>
    </div>
  </div>
  {#if input && aschema && selectedChannel}
    <MinMax
      bind:low={input.lmin}
      bind:high={input.lmax}
      value={sensorValue}
      mode={isContinuousMode(input) || isHisteresisMode(input)
        ? "double"
        : "single"}
      cursorActive={withinWindow}
      min={aschema.min}
      bind:max={maxSensorValue}
      step={aschema.step}
      minLabel={`LowLim (${aschema.unit})`}
      maxLabel={`HighLim (${aschema.unit})`}
    />
  {/if}
  {#if selectedChannel && aschema.cat === "Touch"}
    <div class="row centered">
      <button class="rounded secondary"> Calibrate zero </button>
      <button class="rounded secondary"> Remove offset </button>
      <InfoModal>Information about calibration</InfoModal>
    </div>
  {/if}
{/if}

<style scoped>
  .label {
    text-align: left;
  }
  .buttons {
    display: flex;
    align-items: center;
    gap: 6px;
  }
</style>
