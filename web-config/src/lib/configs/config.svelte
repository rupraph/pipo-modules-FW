<script lang="ts" generics="T extends PipoTypes">
  import { pipoio } from "../../pipoio";
  import { onMount } from "svelte";
  import { schema } from "../../schema";
  import {
    configValid,
    pipoType as type,
    currentConfig,
    activeConfigName,
  } from "../../services";
  import { uiState } from "../ui-state";
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
  import MidiConfigForm from "./midi-config.svelte";
  import OscConfigForm from "./osc-config.svelte";
  import QuickConfig from "./quick-config.svelte";
  import OscGlobalConfig from "./osc-global-config.svelte";
  import SensorModes from "./sensor-modes.svelte";
  import BoardConfig from "./board-config.svelte";
  import Switch from "../form/Switch.svelte";
  import Text from "../form/Text.svelte";
  import Tooltip from "../tooltip/Tooltip.svelte";
  import axios from "axios";
  import Presets from "../presets.svelte";

  // Use stores instead of props
  $: config = $currentConfig as unknown as PipoConfig<T>;
  $: name = $activeConfigName;
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

      // Try to restore selected channel from persisted state
      const persistedChannel =
        $type !== "unknown" ? uiState.getSelectedChannel($type) : undefined;
      const firstChannel = Object.keys(configByChannel)[0] as PipoKeys[T];

      if (persistedChannel && persistedChannel in configByChannel) {
        setAxis(persistedChannel as PipoKeys[T]);
      } else {
        setAxis(firstChannel);
      }
    }
  });

  // Ensures configByChannel updates reactively
  $: if (config) {
    updateConfigByChannel();
    // ConfigSave will auto-update via store subscription in Phase 4
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

      // Persist selected channel to state
      if ($type !== "unknown") {
        uiState.setSelectedChannel($type, axis as string);
      }
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

  function cal_offset(axis: PipoKeys[T]) {
    pipoio.get("/offsetcal", { params: { axis } }).then(({ data }) => {
      config.inputs[axis].offset = data;
    });
  }

  function offsetalltouch() {
    pipoio.get("/offsetAllTouch").then(({ data }) => {
      for (const [axis, offset] of Object.entries(data)) {
        config.inputs[axis as PipoKeys[T]].offset = Number(offset);
      }
    });
  }

  function reset_offset(axis: PipoKeys[T]) {
    axios({
      method: "post",
      url: "/resetoffset",
      params: { axis },
    }).then(() => console.log("DONE"));
  }
</script>

{#if $type !== "range"}
  <Collapse title="Quick settings" collapseId="quick-settings">
    <QuickConfig bind:config />

    {#if $type === "analog"}
      <button class="secondary" on:click={offsetalltouch} title="Zero the touch"
        >Zero All Touch
      </button>
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
<Collapse title="Presets" collapseId="presets">
  <Presets />
</Collapse>

<Collapse title="Channel settings" collapseId="channel-settings" open>
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
      {#if $type !== "motion"}
        <Tooltip title="Make current value the zero offset">
          <button
            class="secondary"
            on:click={() => cal_offset(currentAxis)}
            style="border-radius: 2vw; cursor: pointer;"
          >
            Set Zero
          </button>
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
  <Collapse
    title="OSC settings"
    collapseId="osc-settings"
    bind:value={config.general.OSC_ENA}
  >
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

<Collapse title="Board settings" collapseId="board-settings">
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
