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
  import PillSwitch from "../form/PillSwitch.svelte";
  import { getDefaultDeadband } from "../../defaults";

  $: config = $currentConfig;
  $: type = $pipoType;

  // Get category from uiState instead of local variable
  $: category = ($uiState[type]?.channelType as "analog" | "touch") || "analog";
  $: mode = config?.general.MidiEnabled
    ? "MIDI"
    : config?.general.OSC_ENA
      ? "OSC"
      : "MIDI";

  // Initialize board state when type changes (ensures range board gets default channel)
  $: if (type && type !== "unknown") {
    uiState.initializeBoardState(type);
  }

  $: console.log(
    "Mode changed:",
    mode,
    "MidiEnabled:",
    config?.general.MidiEnabled,
    "OSC_ENA:",
    config?.general.OSC_ENA,
  );

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
  let smoothValue: SmoothSensorValue = {
    new: 0,
    old: 0,
    dt: 0,
    timestamp: Date.now(),
  };
  let maxSensorValue = 100;

  // Conversion constants for analog boards (voltage to percentage)
  const ANALOG_VOLTAGE_MAX = 3.1;

  // Convert voltage to percentage for analog boards (rounded to 1 decimal)
  function voltageToPercent(voltage: number): number {
    return Math.round((voltage / ANALOG_VOLTAGE_MAX) * 100 * 10) / 10;
  }

  // Convert percentage to voltage for analog boards (rounded to 2 decimals)
  function percentToVoltage(percent: number): number {
    return Math.round((percent / 100) * ANALOG_VOLTAGE_MAX * 100) / 100;
  }

  // Reactive variables for analog conversion
  // These provide the UI values (percentage) while storing actual values (voltage)
  $: displaySensorValue =
    type === "analog" && category === "analog" && sensorValue !== undefined
      ? voltageToPercent(sensorValue)
      : sensorValue;

  $: displayLmin =
    type === "analog" && category === "analog" && input?.lmin !== undefined
      ? voltageToPercent(input.lmin)
      : input?.lmin;

  $: displayLmax =
    type === "analog" && category === "analog" && input?.lmax !== undefined
      ? voltageToPercent(input.lmax)
      : input?.lmax;

  // Setters for analog conversion - convert from UI percentage back to voltage
  function setDisplayLmin(value: number) {
    if (!input) return;
    input.lmin =
      type === "analog" && category === "analog"
        ? percentToVoltage(value)
        : value;
  }

  function setDisplayLmax(value: number) {
    if (!input) return;
    input.lmax =
      type === "analog" && category === "analog"
        ? percentToVoltage(value)
        : value;
  }

  // Reactive: Reset maxSensorValue and sensor readings when channel changes
  $: if (selectedChannel && aschema) {
    maxSensorValue = aschema.max;
    // Reset sensor value to schema min to prevent stale readings from affecting maxSensorValue
    // Using schema min instead of undefined prevents layout shifts
    sensorValue = aschema.min;
    smoothValue = {
      new: aschema.min,
      old: aschema.min,
      dt: 0,
      timestamp: Date.now(),
    };
  }

  // Ensure maxSensorValue is always greater than the low value
  $: if (input && input.lmin > maxSensorValue) {
    maxSensorValue = input.lmin + 2000;
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

  // $: console.log("Channel Settings State:", {
  //   config: !!config,
  //   selectedChannel,
  //   channelConfig: !!channelConfig,
  //   type,
  //   uiState: $uiState,
  // });

  $: if (
    sensorValue !== undefined &&
    aschema &&
    sensorValue > maxSensorValue &&
    sensorValue > aschema.max
  ) {
    maxSensorValue = Math.round(sensorValue);
  }

  function toggleRangeInvert() {
    if (!input) return;
    input.inverted = !input.inverted;
    currentConfig.set(config);
  }

  function toggleBinaryMode() {
    if (!input || !aschema) return;

    // For Touch category: toggle only mode
    if (aschema.cat === "Touch") {
      if (input.mode === false && input.th_mode === false) {
        // Currently continuous -> switch to binary (threshold mode only)
        input.mode = true;
        input.th_mode = false;
      } else {
        // Currently binary -> switch to continuous
        input.mode = false;
        input.th_mode = false;
      }
    } else {
      // For other categories: toggle both mode and th_mode together
      // Binary enabled: both true
      // Binary disabled (continuous): both false
      if (input.mode === true && input.th_mode === true) {
        // Currently binary -> switch to continuous (both false)
        input.mode = false;
        input.th_mode = false;
      } else {
        // Currently continuous or other -> switch to binary (both true)
        input.mode = true;
        input.th_mode = true;
      }
    }

    currentConfig.set(config);
  }

  function toggleCyclic() {
    if (!input) return;
    input.cyclic = !input.cyclic;
    currentConfig.set(config);
  }

  function toggleOverOut() {
    if (!input) return;
    input.over_out = !input.over_out;
    currentConfig.set(config);
  }

  // Default and current state for the noise filter (deadband)
  $: defaultDeadband = selectedChannel
    ? getDefaultDeadband(type, selectedChannel)
    : 0;
  $: deadbandEnabled = input ? input.deadband !== 0 : false;

  function handleDeadbandToggle() {
    if (!input) return;
    input.deadband = deadbandEnabled ? 0 : defaultDeadband;
  }

  // Compute if binary mode is active
  $: isBinaryMode = input
    ? aschema?.cat === "Touch"
      ? input.mode === true
      : input.mode === true && input.th_mode === true
    : false;

  // Compute if over mode is active
  $: isOverOut = input ? input.over_out === true : false;

  // Compute if inverted is active
  $: isInverted = input ? input.inverted === true : false;

  // Check if we're in MIDI Note mode
  $: isNoteMode =
    mode === "MIDI" &&
    channelConfig &&
    "tl_mode" in channelConfig &&
    channelConfig.tl_mode === 1;

  // Check if current channel is an Euler angle (for motion board)
  $: isEulerAngle =
    type === "motion" &&
    selectedChannel &&
    ["yaw", "pitch", "roll"].includes(selectedChannel);

  // Calibration state
  let calibrating = false;

  function calibrateZero() {
    if (!selectedChannel || calibrating) return;

    calibrating = true;

    pipoio
      .post("/offsetcal", null, { params: { axis: selectedChannel } })
      .then(({ data }) => {
        if (data.status === "measuring") {
          // Start polling for completion
          pollCalibrationCompletion();
        }
      })
      .catch((error) => {
        console.error("Offset calibration failed:", error);
        calibrating = false;
      });
  }

  function pollCalibrationCompletion() {
    const pollInterval = setInterval(() => {
      pipoio
        .get("/offsetcal-status")
        .then(({ data }) => {
          if (data.status === "complete" && data.offsets) {
            // Update the offset value for the current channel
            if (selectedChannel && input) {
              const offsetValue = data.offsets[selectedChannel];
              if (offsetValue !== undefined) {
                input.offset = offsetValue as number;
                console.log(
                  `Offset calibration completed for ${selectedChannel}:`,
                  offsetValue,
                );
                // Trigger config update
                if (config) {
                  currentConfig.set(config);
                }
              }
            }
            clearInterval(pollInterval);
            calibrating = false;
          }
          // Continue polling if still measuring
        })
        .catch((error) => {
          console.error("Offset status check failed:", error);
          clearInterval(pollInterval);
          calibrating = false;
        });
    }, 500); // Poll every 500ms

    // Timeout after 10 seconds
    setTimeout(() => {
      clearInterval(pollInterval);
      if (calibrating) {
        console.error("Offset calibration timeout");
        calibrating = false;
      }
    }, 10000);
  }

  function removeOffset() {
    if (!selectedChannel) return;

    pipoio
      .post("/resetoffset", null, { params: { axis: selectedChannel } })
      .then(() => {
        if (input) {
          input.offset = 0;
          console.log(`Offset reset for ${selectedChannel}`);
          // Trigger config update
          if (config) {
            currentConfig.set(config);
          }
        }
      })
      .catch((error) => {
        console.error("Reset offset failed:", error);
      });
  }
</script>

{#if config && selectedChannel && channelConfig}
  {#if type !== "range"}
    <div class="row">
      <span class="label">Channel state</span>
      <div class="buttons channel-state">
        <PillSwitch
          label=""
          bind:value={channelConfig.enabled}
          offLabel="OFF"
          onLabel="ON"
          on:change={() => {
            currentConfig.set(config);
          }}
        />
      </div>
    </div>
  {/if}
  {#if mode === "OSC"}
    <div class="row">
      <span class="label">Raw output</span>
      <div class="buttons">
        <PillSwitch
          label=""
          bind:value={channelConfig.mode_raw}
          on:change={() => {
            currentConfig.set(config);
          }}
        />
      </div>
    </div>
  {/if}
  {#if !channelConfig.mode_raw && !(aschema.cat === "Touch")}
    <div class="row">
      <div class="left">
        <span class="label">Options</span>
        <InfoModal>
          <p><u>Invert:</u> This inverts the output range of the channel.</p>
          <p>
            <u>Binary mode:</u> When enabled, the channel outputs only a true or
            false state. Truewithin the sliders, false outside. When disabled, the
            channel outputs continuous values.
          </p>
          {#if type !== "motion" && type !== "analog" && !(type === "range" && config?.sensorconf && "hold_mode" in config.sensorconf && config.sensorconf.hold_mode)}
            <p>
              <u>Over mode:</u> When enabled, if the reading exceeds the maximum
              slider value, the output is set to 0. Otherwise, it is clamped to the
              max value.
            </p>
          {/if}
          {#if type !== "range" && (type !== "motion" || isEulerAngle)}
            <p>
              <u>Cyclic:</u> When enabled, the output wraps within the min max slider,
              allowing for continous cycling: output will be 0 when at min and at
              max. maximum is reached in the middle of the range.
            </p>
          {/if}
        </InfoModal>
      </div>

      <div class="buttons">
        <button
          class="rounder primary"
          class:enabled={input?.inverted}
          on:click={toggleRangeInvert}
        >
          Invert
        </button>
        <button
          class="rounder primary"
          class:enabled={isBinaryMode}
          on:click={toggleBinaryMode}
        >
          Binary mode
        </button>
        {#if type !== "motion" && type !== "analog" && !(type === "range" && config?.sensorconf && "hold_mode" in config.sensorconf && config.sensorconf.hold_mode)}
          <button
            class="rounder primary"
            class:enabled={isOverOut}
            class:disabled={isBinaryMode || isInverted || isNoteMode}
            disabled={isBinaryMode || isInverted || isNoteMode}
            on:click={toggleOverOut}
          >
            Over mode
          </button>
        {/if}
        {#if type !== "range" && (type !== "motion" || isEulerAngle)}
          <div class="buttons">
            <button
              class="rounder primary"
              class:enabled={input?.cyclic}
              class:disabled={isBinaryMode}
              disabled={isBinaryMode}
              on:click={toggleCyclic}
            >
              Cyclic
            </button>
          </div>
        {/if}
      </div>
    </div>
  {/if}
  {#if input && aschema && selectedChannel}
    {#if type === "analog" && category === "analog"}
      <MinMax
        low={displayLmin}
        high={displayLmax}
        on:lowChange={(e) => setDisplayLmin(e.detail)}
        on:highChange={(e) => setDisplayLmax(e.detail)}
        value={displaySensorValue}
        mode={isContinuousMode(input) || isHisteresisMode(input)
          ? "double"
          : "single"}
        cursorActive={withinWindow}
        min={0}
        max={100}
        step={1}
        minLabel={`Min`}
        maxLabel={`Max`}
        units={"%"}
      />
    {:else}
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
        minLabel={`Min`}
        maxLabel={`Max`}
        units={aschema.unit}
      />
    {/if}
  {/if}
  {#if mode === "OSC"}
    <div class="row">
      <div class="left">
        <span class="label">Noise Filter</span>
        <InfoModal>
          <p>
            This filter removes small sensor variations to reduce network
            traffic. Disable for full sensitivity, unfiltered readings.
          </p>
        </InfoModal>
      </div>
      <PillSwitch
        label=""
        value={deadbandEnabled}
        on:change={handleDeadbandToggle}
      />
    </div>
  {/if}

  <!-- {#if selectedChannel && aschema.cat === "Touch"}
    <div class="row centered">
      <button
        class="rounded secondary"
        on:click={calibrateZero}
        disabled={calibrating}
      >
        {calibrating ? "Calibrating..." : "Calibrate zero"}
      </button>
      <button
        class="rounded secondary"
        on:click={removeOffset}
        disabled={calibrating}
      >
        Remove offset
      </button>
      <InfoModal>
        <p style="white-space: normal;">
          Touch inputs are very sensitive and are heavily affected by what you
          will connect to it or its surroundings, requiring systematic
          calibration whenever you change something in your setup. Pipo
          calibrates the "no touch" (zero) reference at every startup to help
          you with this. You can then adjust "touch" sensitivity by changing the
          threshold with the channel slider.
        </p>
        <p style="white-space: normal;">
          Calibrate zero: Measures the current touch reading and use it as "no
          touch" reference (zero).
        </p>
        <p style="white-space: normal;">
          Remove offset: Removes any calibration offset, value is the raw sensor
          readings.
        </p>
      </InfoModal>
    </div>
  {/if} -->
{/if}

<style scoped>
  .label {
    text-align: left;
    white-space: nowrap;
  }
  .buttons {
    max-width: 335px;
    display: flex;
    align-content: flex-start;
    /* align-items: center; */
    gap: 6px;
    flex-wrap: wrap;
  }

  /* Make the channel state pill switch larger */
  .buttons.channel-state :global(.pill-switch) {
    min-width: 80px;
  }

  .buttons.channel-state :global(.pill-switch label) {
    padding: 0px 8px;
    font-size: 12px;
  }

  button.disabled {
    opacity: 0.4;
    cursor: not-allowed;
    pointer-events: none;
  }
</style>
