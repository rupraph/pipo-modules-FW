<script lang="ts" generics="T extends PipoTypes">
  import { pipoio } from "../../pipoio";

  import HidGlobalConfig from "./hid-global-config.svelte";

  import { createEventDispatcher, onMount } from "svelte";
  import { schema } from "../../schema";
  import { pipoType as type } from "../../services";
  import Select from "svelte-select";
  import {
    type SensorConfig,
    type PipoConfig,
    type PipoTypes,
    type PipoKeys,
    type ConfigByAxis,
    type AxisSchema,
    type MidiConfig,
    type OscConfig,
    type HidConfig,
  } from "../../types";
  import axios from "axios";
  import Collapse from "../collapse.svelte";
  import LoadingButton from "../form/LoadingButton.svelte";
  import AxisConfig from "./axis-config.svelte";
  import CategoryTab from "./category-tab.svelte";
  import HidConfigForm from "./hid-config.svelte";
  import MidiConfigForm from "./midi-config.svelte";
  import OscConfigForm from "./osc-config.svelte";
  import QuickConfig from "./quick-config.svelte";
  import OscGlobalConfig from "./osc-global-config.svelte";
  import BoardConfig from "./board-config.svelte";
  export let config: PipoConfig<T>;
  export let name: string;
  const dispatch = createEventDispatcher();
  let savingStatus = "none";

  let configByAxis: ConfigByAxis<T>;
  let currentAxis: PipoKeys[T];
  let midi: MidiConfig;
  let osc: OscConfig;
  let hid: HidConfig;
  let sensor: SensorConfig;
  let aschema: AxisSchema;
  let axisSelect: { value: string; label: string }[] = [];
  let currentCat = "MIDI";
  onMount(() => {
    configByAxis = (
      Object.entries(config.sensor) as [PipoKeys[T], SensorConfig][]
    )
      .sort(
        (a, b) =>
          schema[$type as T][a[0]].index - schema[$type as T][b[0]].index
      )
      .reduce((acc, [axis, sensor]) => {
        acc[axis] = {
          sensor,
          hid: config.engine["engine-hid"][axis],
          midi: config.engine["engine-midi"][axis],
          osc: config.engine["engine-osc"][axis],
        };
        return acc;
      }, {} as ConfigByAxis<T>);
    axisSelect = Object.keys(configByAxis).map((axis) => {
      return { value: axis, label: schema[$type as T][axis].label };
    });
    setAxis(Object.keys(configByAxis)[0]);
  });

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

  function download() {
    const data = JSON.stringify(config, 0, 2);
    const blob = new Blob([data], { type: "application/json" });
    const url = URL.createObjectURL(blob);
    const a = document.createElement("a");
    a.download = "config.json";
    a.target = "_blank";
    a.href = url;
    a.click();
  }

  let isPaused = false;
  function pause() {
    pipoio.post("/pause").then(() => {
      console.log("Pausing...");
    });
    isPaused = !isPaused;
  }

  function setAxis(axis: PipoKeys[T]) {
    if (axis === currentAxis) return;
    currentAxis = axis;
    midi = configByAxis[axis].midi;
    osc = configByAxis[axis].osc;
    hid = configByAxis[axis].hid;
    aschema = schema[$type as T][axis];
    sensor = configByAxis[axis].sensor;
    pipoio.monitorAxis(axis);
  }
  function setCategory(cat: string) {
    currentCat = cat;
  }
  let interval = 0;
  // onMount(() => {
  //   interval = window.setInterval(() => {
  //     configSave.update(JSON.parse(JSON.stringify(config)));
  //   }, 1000);
  // });
  // onDestroy(() => {
  //   clearInterval(interval);
  // });

  /**
 


 */
  $: if (config && currentAxis) {
    setAxis(currentAxis);
  }
</script>

<Collapse title="Quick settings">
  <QuickConfig bind:config schema={schema[$type]} />
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

<Collapse title="Axis settings" open>
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
    </div>

    <AxisConfig bind:sensor bind:aschema bind:currentAxis />
    <!-- <Checkbox label="Inverted" bind:value={sensorconf.inverted} /> -->

    <CategoryTab active={currentCat} onClick={(cat) => setCategory(cat)} />
    <section class="translator-settings">
      {#if currentCat === "MIDI"}
        <MidiConfigForm {midi} bind:sensormode={sensor.mode} />
      {/if}
      {#if currentCat === "HID"}
        <HidConfigForm
          bind:hidEnabled={config.general.HidEnabled}
          bind:hidMode={config.general.HidMode}
          {sensor}
          {hid}
        />
      {/if}
      {#if currentCat === "OSC"}
        <OscConfigForm {osc} />
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
</Collapse>

<hr class="separator" />

<Collapse title="OSC settings" bind:value={config.general.OSC_ENA}>
  <OscGlobalConfig
    bind:ip={config.general.OSC_IP}
    bind:port={config.general.OSC_PORT}
  />
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

<hr class="separator" />
<Collapse title="Board settings"
  ><BoardConfig bind:wifiMode={config.general.Wifi_mode} />
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
