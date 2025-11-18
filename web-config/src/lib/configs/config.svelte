<script lang="ts" generics="T extends PipoTypes">
  import Radio from "../form/Radio.svelte";

  import { pipoio } from "../../pipoio";
  import HidGlobalConfig from "./hid-global-config.svelte";
  import { onMount } from "svelte";
  import { schema } from "../../schema";
  import { configSave, configValid, pipoType as type } from "../../services";
  import Select from "svelte-select";
  import {
    type InputSettings,
    type PipoConfig,
    type PipoTypes,
    type PipoKeys,
    type ConfigByChannel,
    type AxisSchema,
    type MidiConfig,
    type OscConfig,
    type HidConfig,
  } from "../../types";
  import Collapse from "../collapse.svelte";
  import LoadingButton from "../form/LoadingButton.svelte";
  import InputConfig from "./input-panel.svelte";
  import CategoryTab from "./category-tab.svelte";
  import HidConfigForm from "./hid-config.svelte";
  import MidiConfigForm from "./midi-config.svelte";
  import OscConfigForm from "./osc-config.svelte";
  import QuickConfig from "./quick-config.svelte";
  import AnalogOutForm from "./analog-out.svelte";
  import OscGlobalConfig from "./osc-global-config.svelte";
  import SensorModes from "./sensor-modes.svelte";
  import BoardConfig from "./board-config.svelte";
  import Switch from "../form/Switch.svelte";
  import Text from "../form/Text.svelte";
  import Tooltip from "../tooltip/Tooltip.svelte";
  import axios from "axios";

  export let config: PipoConfig<T>;
  export let name: string;
  let savingStatus = "none";

  let configByChannel: ConfigByChannel<T> = {} as ConfigByChannel<T>;
  let currentAxis: PipoKeys[T];
  let midi: MidiConfig;
  let osc: OscConfig;
  let hid: HidConfig;
  let input: InputSettings;
  let aschema: AxisSchema;
  let axisSelect: { value: string; label: string }[] = [];
  let currentCat = "MIDI";
  let isConfigValid = false;

  configValid.subscribe((valid) => {
    isConfigValid = valid;
  });
  //  subscribe to the confgiValid store
  onMount(() => {
    if (config) {
      updateConfigByChannel();
      setAxis(Object.keys(configByChannel)[0] as PipoKeys[T]);
    }
  });

  // Ensures configByChannel updates reactively
  $: if (config) {
    updateConfigByChannel();
    // @ts-expect-error
    configSave.update(config);
  }

  $: if (config.general.MidiEnabled) {
    currentCat = "MIDI";
  } else {
    currentCat = "OSC";
  }

  function updateConfigByChannel() {
    configByChannel = (
      Object.entries(config.inputs) as [PipoKeys[T], InputSettings][]
    )
      .sort(
        (a, b) =>
          schema[$type as T][a[0]].index - schema[$type as T][b[0]].index
      )
      .reduce((acc, [axis, input]) => {
        acc[axis] = {
          input,
          hid: config.engine["engine-hid"][axis],
          midi: config.engine["engine-midi"][axis],
          osc: config.engine["engine-osc"][axis],
        };
        return acc;
      }, {} as ConfigByChannel<T>);

    axisSelect = (Object.keys(configByChannel) as PipoKeys[T][]).map(
      (axis) => ({
        value: axis,
        label: schema[$type as T][axis].label,
      })
    );
  }
  let hide_on_out = false;
  function isDisabled(
    currentCat: string,
    midi: MidiConfig,
    hid: HidConfig,
    osc: OscConfig
  ) {
    const cat = currentCat === "MIDI" ? midi : currentCat === "HID" ? hid : osc;
    return !cat.enabled;
  }

  // Ensures `midi`, `osc`, `hid`, etc. update when `currentAxis` changes
  $: if (configByChannel && currentAxis) {
    midi = configByChannel[currentAxis].midi;
    osc = configByChannel[currentAxis].osc;
    hid = configByChannel[currentAxis].hid;
    aschema = schema[$type as T][currentAxis];
    input = configByChannel[currentAxis].input;
    if (aschema.cat === "Voltage") {
      if (config.sensorconf.analogout[currentAxis].pindir) {
        hide_on_out = true;
      } else {
        hide_on_out = false;
      }
    } else {
      hide_on_out = false;
    }
  }

  function setAxis(axis: PipoKeys[T]) {
    if (axis !== currentAxis) {
      currentAxis = axis;
      pipoio.monitorAxis(axis);
    }
  }

  function submit() {
    console.log("Saving...");
    savingStatus = "loading";
    const blob = new Blob([JSON.stringify(config)], {
      type: "application/json",
    });
    const formData = new FormData();
    formData.append("file", blob, name);
    Promise.all([
      new Promise((resolve) => setTimeout(resolve, 1000)),
      pipoio.request({
        method: "post",
        url: "/save",
        data: formData,
        headers: { "Content-Type": "multipart/form-data" },
      }),
    ])
      .then(() => {
        console.log("Saved...");
        savingStatus = "success";
      })
      .catch(() => {
        savingStatus = "error";
      })
      .finally(() => {
        setTimeout(() => {
          savingStatus = "none";
        }, 1000);
      });
  }

  function reboot() {
    pipoio.get("/reboot").then(() => console.log("Rebooting..."));
  }
  let isPaused = false;
  function pause() {
    pipoio.post("/pause").then(() => {
      console.log("Pausing...");
    });
    isPaused = !isPaused;
  }

  // Offset calibration states
  let calibratingAxis: string | null = null;
  let calibratingAll = false;

  function cal_offset(axis: PipoKeys[T]) {
    calibratingAxis = axis;
    pipoio
      .post("/offsetcal", null, { params: { axis } })
      .then(({ data }) => {
        if (data.status === "measuring") {
          // Start polling for completion
          pollOffsetCompletion(axis);
        }
      })
      .catch((error) => {
        console.error(`Offset calibration failed for ${axis}:`, error);
        calibratingAxis = null;
      });
  }

  function pollOffsetCompletion(axis: PipoKeys[T]) {
    const pollInterval = setInterval(() => {
      pipoio
        .get("/offsetcal-status")
        .then(({ data }) => {
          if (data.status === "complete" && data.offsets) {
            // Use the returned offset values directly
            if (data.offsets[axis] !== undefined) {
              config.inputs[axis].offset = data.offsets[axis];
              console.log(
                `Offset calibrated for ${axis}: ${data.offsets[axis]}`
              );
            }
            clearInterval(pollInterval);
            calibratingAxis = null;
          }
          // Continue polling if still measuring
        })
        .catch((error) => {
          console.error(`Offset status check failed for ${axis}:`, error);
          clearInterval(pollInterval);
          calibratingAxis = null;
        });
    }, 500); // Poll every 500ms

    // Timeout after 10 seconds
    setTimeout(() => {
      clearInterval(pollInterval);
      if (calibratingAxis === axis) {
        console.error(`Offset calibration timeout for ${axis}`);
        calibratingAxis = null;
      }
    }, 10000);
  }

  function pollAllTouchCompletion() {
    const pollInterval = setInterval(() => {
      pipoio
        .get("/offsetcal-status")
        .then(({ data }) => {
          if (data.status === "complete" && data.offsets) {
            // Use the returned offset values directly
            for (const [channel, offsetValue] of Object.entries(data.offsets)) {
              if (config.inputs[channel as PipoKeys[T]]) {
                config.inputs[channel as PipoKeys[T]].offset =
                  offsetValue as number;
              }
            }
            console.log(
              "All touch offset calibration completed:",
              data.offsets
            );
            clearInterval(pollInterval);
            calibratingAll = false;
          }
          // Continue polling if still measuring
        })
        .catch((error) => {
          console.error("All touch offset status check failed:", error);
          clearInterval(pollInterval);
          calibratingAll = false;
        });
    }, 500); // Poll every 500ms

    // Timeout after 10 seconds
    setTimeout(() => {
      clearInterval(pollInterval);
      if (calibratingAll) {
        console.error("All touch offset calibration timeout");
        calibratingAll = false;
      }
    }, 10000);
  }

  function offsetalltouch() {
    calibratingAll = true;

    // Get all touch channel names from schema
    const touchChannels = (Object.keys(configByChannel) as PipoKeys[T][])
      .filter((axis) => schema[$type as T][axis].cat === "Touch")
      .join(",");

    pipoio
      .post("/offsetcal-list", null, { params: { channels: touchChannels } })
      .then(({ data }) => {
        if (data.status === "measuring") {
          // Start polling for completion
          pollAllTouchCompletion();
        }
      })
      .catch((error) => {
        console.error("Touch offset calibration failed:", error);
        calibratingAll = false;
      });
  }

  function reset_offset(axis: PipoKeys[T]) {
    pipoio
      .request({
        method: "post",
        url: "/resetoffset",
        params: { axis },
      })
      .then(() => {
        // Set local config to 0
        config.inputs[axis].offset = 0;
        console.log(`Offset reset for ${axis}`);
      })
      .catch((error) => {
        console.error(`Failed to reset offset for ${axis}:`, error);
      });
  }

  function reset_all_offsets() {
    pipoio
      .request({
        method: "post",
        url: "/resetoffset",
      })
      .then(() => {
        // Set all local config offsets to 0
        for (const axis of Object.keys(config.inputs) as PipoKeys[T][]) {
          config.inputs[axis].offset = 0;
        }
        console.log("All offsets reset");
      })
      .catch((error) => {
        console.error("Failed to reset all offsets:", error);
      });
  }
</script>

{#if $type !== "range"}
  <Collapse title="Quick settings">
    <QuickConfig bind:config />

    {#if $type === "analog"}
      <LoadingButton
        onClick={offsetalltouch}
        loading={calibratingAll}
        disabled={calibratingAxis !== null}
        class="secondary"
        title="Zero all touch sensors"
      >
        {calibratingAll ? "Calibrating All..." : "Zero All Touch"}
      </LoadingButton>
      <!-- <button
        class="secondary"
        on:click={reset_all_offsets}
        title="Reset all offsets to zero"
        style="border-radius: 2vw; cursor: pointer;"
      >
        Reset All
      </button> -->
    {/if}

    <button
      class="primary Pause"
      on:click={pause}
      title="Pause sending data"
      style="margin: 20px;"
    >
      {#if isPaused}
        Resume
      {/if}
      {#if !isPaused}
        Pause all output
      {/if}
    </button>

    <LoadingButton
      onClick={submit}
      loading={savingStatus === "loading"}
      disabled={!isConfigValid}
      class={savingStatus === "success"
        ? "success"
        : savingStatus === "error"
          ? "error"
          : "primary"}
      title="Apply and save the config in pipo">Save</LoadingButton
    >
  </Collapse>
  <hr class="separator" />
{/if}

<Collapse title="Channel settings" open>
  {#if currentAxis && config}
    <div class="axis-selector">
      <h4>Input:</h4>
      <Select
        items={axisSelect}
        clearable={false}
        searchable={false}
        class="axis-select"
        value={currentAxis}
        --selected-item-color="var(--text-color)"
        --font-size="20px"
        --item-is-active-bg="var(--bg-lighter)"
        --item-color="var(--text-color)"
        --item-bg="var(--bg-secondary)"
        --input-color="var(--text-color-secondary)"
        --item-hover-color="var(--text-color)"
        --item-hover-bg="var(--bg-lighter)"
        --border-radius="30px"
        --border="0"
        --width="35%"
        --border-focused="0"
        --list-background="var(--bg-secondary)"
        --background="var(--bg-tabs)"
        on:change={(evt) => setAxis(evt.detail.value)}
      />
      {#if $type == "analog" && aschema.cat == "Touch"}
        <Tooltip title="Make current value the zero offset">
          <LoadingButton
            onClick={() => cal_offset(currentAxis)}
            loading={calibratingAxis === currentAxis}
            disabled={calibratingAxis !== null &&
              calibratingAxis !== currentAxis}
            class="secondary"
            style="border-radius: 2vw; cursor: pointer;"
          >
            {calibratingAxis === currentAxis ? "Calibrating..." : "Set Zero"}
          </LoadingButton>
        </Tooltip>
        <Tooltip title="Removes the offset">
          <button
            class="secondary"
            on:click={() => reset_offset(currentAxis)}
            style="border-radius: 2vw; cursor: pointer;"
          >
            Reset Zero
          </button>
        </Tooltip>
      {/if}
    </div>
    {#if hide_on_out}
      <p>This channel is currently used for output</p>
      <p>Check beta section below</p>
    {:else}
      <InputConfig bind:input bind:aschema bind:currentAxis />
      <!-- <CategoryTab active={currentCat} onClick={setCategory} /> -->
      <Tooltip
        title="Disabled in Quick config"
        followCursor={true}
        enabled={isDisabled(currentCat, midi, hid, osc)}
      >
        <section
          class="translator-settings"
          class:not-allowed={isDisabled(currentCat, midi, hid, osc)}
        >
          <!-- <h4>{currentCat} output settings</h4> -->
          <span class="translator-title"> {currentCat} output settings</span>
          <hr class="separator" />

          {#if currentCat === "MIDI"}
            <MidiConfigForm bind:midi bind:sensormode={input.mode} />
          {/if}
          <!-- {#if currentCat === "HID"}
            <HidConfigForm
              bind:hidMode={config.general.HidMode}
              bind:input
              {hid}
            />
          {/if} -->
          {#if currentCat === "OSC"}
            <OscConfigForm bind:osc />
          {/if}
        </section>
      </Tooltip>

      <div style="display:flex; margin-top:1em; justify-content:right;">
        <LoadingButton
          onClick={submit}
          loading={savingStatus === "loading"}
          class={savingStatus === "success"
            ? "success"
            : savingStatus === "error"
              ? "error"
              : "primary"}
          title="Apply and save the config in pipo">Save</LoadingButton
        >
      </div>
    {/if}
  {/if}
</Collapse>
<hr class="separator" />
{#if config.sensorconf}
  <SensorModes bind:config={config.sensorconf} />
  <hr class="separator" />
{/if}

{#if config.general.OSC_ENA}
  <Collapse title="OSC settings" bind:value={config.general.OSC_ENA}>
    <section class="OSC-global-settings">
      <OscGlobalConfig
        bind:ip={config.general.OSC_IP}
        bind:port={config.general.OSC_PORT}
      />
    </section>
    {#if $type === "motion"}
      {#if config.engine["engine-special"] && config.engine["engine-special"]["quat"]}
        <Switch
          label="MOTION: Quaternions to OSC"
          bind:value={config.engine["engine-special"]["quat"].enabled}
          design="slider"
        />
        {#if config.engine["engine-special"]["quat"].enabled}
          <Text
            label="Address"
            bind:value={config.engine["engine-special"]["quat"].osc_addr}
          />
        {/if}
      {/if}
    {/if}
    <div style="display:flex; margin-top:1em; justify-content:right;">
      <LoadingButton
        onClick={submit}
        loading={savingStatus === "loading"}
        disabled={!isConfigValid}
        class={savingStatus === "success"
          ? "success"
          : savingStatus === "error"
            ? "error"
            : "primary"}
        title="Apply and save the config in pipo">Save</LoadingButton
      >
    </div>
  </Collapse>
  <hr class="separator" />
{/if}

<!-- <hr class="separator" />
<Collapse title="Beta Features">
  {#if $type === "analog"}
    <Collapse title="HW Output (from OSC only)">
      <AnalogOutForm bind:outconfig={config.sensorconf} />
    </Collapse>
  {/if}
  <Collapse title="HID settings" bind:value={config.general.HidEnabled}
    ><HidGlobalConfig bind:mode={config.general.HidMode} />
    <div style="display:flex; margin-top:1em; justify-content:right;">
      <LoadingButton
        onClick={submit}
        loading={savingStatus === "loading"}
        disabled={!isConfigValid}
        class={savingStatus === "success"
          ? "success"
          : savingStatus === "error"
            ? "error"
            : "primary"}
        title="Apply and save the config in pipo">Save</LoadingButton
      >
    </div></Collapse
  >
</Collapse> -->

<Collapse title="Board settings">
  <BoardConfig bind:generalconfig={config.general} />
  <div style="display:flex; margin-top:1em; justify-content:right;">
    <LoadingButton
      onClick={submit}
      loading={savingStatus === "loading"}
      disabled={!isConfigValid}
      class={savingStatus === "success"
        ? "success"
        : savingStatus === "error"
          ? "error"
          : "primary"}
      title="Apply and save the config in pipo">Save</LoadingButton
    >
  </div>
  <button class="primary" on:click={reboot} style="width: fit-content"
    >Reboot</button
  >
</Collapse>

<style>
  button:hover {
    background-color: var(--main-darker);
  }

  .separator {
    border: 0;
    height: 2px;
    background: var(--bg-lighter);
    margin: 10px 0;
  }

  .Pause {
    background-color: rgb(211, 211, 211);
  }

  .translator-title {
    display: flex;
    align-items: center;
    justify-content: center;
    position: relative;
    font-weight: bold;
    /* padding: 0.8em 0; */
  }

  .translator-settings {
    background-color: var(--bg-tabs);
    padding: 1em;
    /* border-bottom-left-radius: 0.8em;
    border-bottom-right-radius: 0.8em; */
    border-radius: 0.8em;
    padding-left: 3em;
    padding-right: 3em;
  }

  .OSC-global-settings {
    /* background-color: var(--bg-tabs); */
    padding: 1em;
    /* border-bottom-left-radius: 0.8em;
    border-bottom-right-radius: 0.8em; */
    padding-left: 3em;
    padding-right: 4em;
  }

  .axis-selector {
    display: flex;
    flex-direction: row;
    align-items: center;
    justify-content: center;
    margin-bottom: 1.5em;
    gap: 1em;
  }
</style>
