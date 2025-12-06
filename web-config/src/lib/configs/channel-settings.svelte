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
            sensorValue = smoothValue.old + (smoothValue.new - smoothValue.old) * (smoothValue.dt / 1000);
        }
        requestAnimationFrame(animateSensor);
    }
    animateSensor();

    $: if (sensorValue !== undefined && aschema && sensorValue > maxSensorValue && sensorValue > aschema.max) {
        maxSensorValue = Math.round(sensorValue);
    }

    function isEnabled() {
        // TODO: Rupert where is the muted ? 
        if (!config || !selectedChannel) return false;
        return config.engine[engineKey][selectedChannel].muted;
    }
    function isSolo() {
        // TODO: Rupert where is the solo ? 
        if (!config || !selectedChannel) return false;
        return config.engine[engineKey][selectedChannel].solo;
    }
</script>

{#if config}
    <div class="row">
        <span class="label">Channel state</span>
        <div class="buttons">
            <button class:enabled={isEnabled()}> Mute </button>
            <button class:enabled={!isSolo()}> Solo </button>
        </div>
    </div>
    <div class="row">
        <div class="left">
            <span class="label">Options</span>
            <InfoModal>Information about options</InfoModal>
        </div>
        <div class="buttons">
            <button> Range invert </button>
            <button> Binqry mode </button>
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
{/if}

<style scoped>
    .row {
        font-family: Instrument Sans;
        font-size: 12px;
        font-weight: 700;
        display: flex;
        align-items: center;
        justify-content: space-between;
        box-sizing: content-box;
        width: calc(100% - 44px);
    }
    .left {
        display: flex;
        align-items: center;
        gap: 8px;
    }
    .label {
        text-align: left;
    }
    .buttons {
        display: flex;
        align-items: center;
        gap: 6px;
    }
    .row button {
        border: 2px solid var(--main);
        height: 29px;
        padding: 0 16px;
        background-color: var(--bg-secondary);
        border-radius: 18px;
        font-size: 12px;
        line-height: 16px;
        font-weight: 700;
        color: var(--main);
        cursor: pointer;
    }
</style>
