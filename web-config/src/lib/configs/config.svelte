<script lang="ts" generics="T extends PipoTypes">
  import { pipoio } from "../../pipoio";
  import HidGlobalConfig from "./hid-global-config.svelte";
  import { onMount } from "svelte";
  import { schema } from "../../schema";
  import { pipoType as type } from "../../services";
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

  onMount(() => {
    if (config) {
      updateConfigByChannel();
      setAxis(Object.keys(configByChannel)[0] as PipoKeys[T]);
    }
  });

  // Ensures configByChannel updates reactively
  $: if (config) {
    updateConfigByChannel();
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

  function setCategory(cat: string) {
    currentCat = cat;
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
    axios({
      method: "post",
      url: "/offsetcal",
      params: { axis },
    }).then(() => console.log("DONE"));
  }
</script>

<Collapse title="Quick settings">
  <div style="overflow-x: auto;">
    <QuickConfig bind:config />
  </div>
  {#if $type === "analog"}
    <button
      class="primary"
      on:click={() => {
        pipoio.post("/offsetAllTouch").then(() => {
          console.log("zero all touch");
        });
      }}
      title="Zero the touch"
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
    class={savingStatus === "success"
      ? "success"
      : savingStatus === "error"
        ? "error"
        : "primary"}
    title="Apply and save the config in pipo">Save</LoadingButton
  >
</Collapse>
<hr class="separator" />

<Collapse title="Input settings" open>
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
        --input-color="var(--text-color)"
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

      <Tooltip title="Make current value the zero offset">
        <button
          class="primary"
          on:click={() => cal_offset(currentAxis)}
          style="border-radius: 2vw; cursor: pointer;"
        >
          Zero
        </button>
      </Tooltip>
    </div>
    {#if hide_on_out}
      <p>This channel is currently used for output</p>
      <p>Check beta section below</p>
    {:else}
      <InputConfig bind:input bind:aschema bind:currentAxis />
      <CategoryTab active={currentCat} onClick={setCategory} />

      <section class="translator-settings">
        {#if currentCat === "MIDI"}
          <MidiConfigForm bind:midi bind:sensormode={input.mode} />
        {/if}
        {#if currentCat === "HID"}
          <HidConfigForm
            bind:hidMode={config.general.HidMode}
            bind:input
            {hid}
          />
        {/if}
        {#if currentCat === "OSC"}
          <OscConfigForm bind:osc />
        {/if}
      </section>
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
<SensorModes bind:config={config.sensorconf} />
<hr class="separator" />
<Collapse title="OSC settings" bind:value={config.general.OSC_ENA}>
  <OscGlobalConfig
    bind:ip={config.general.OSC_IP}
    bind:port={config.general.OSC_PORT}
  />
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
        class={savingStatus === "success"
          ? "success"
          : savingStatus === "error"
            ? "error"
            : "primary"}
        title="Apply and save the config in pipo">Save</LoadingButton
      >
    </div></Collapse
  >
</Collapse>

<hr class="separator" />
<Collapse title="Board settings">
  <BoardConfig bind:generalconfig={config.general} />
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

  .translator-settings {
    background-color: var(--bg-tabs);
    padding: 1em;
    border-bottom-left-radius: 0.8em;
    border-bottom-right-radius: 0.8em;
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
