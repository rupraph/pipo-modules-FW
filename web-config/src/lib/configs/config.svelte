<script lang="ts" generics="T extends PipoTypes">
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
    ).reduce((acc, [axis, sensor]) => {
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
      axios({
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
    axios.post("/pause").then(() => {
      console.log("Pausing...");
    });
    isPaused = !isPaused;
  }

  function switchwifimode() {
    axios.post("/wifimode").then(() => {
      console.log("Switching wifi mode...");
    });
  }

  function setAxis(axis: PipoKeys[T]) {
    currentAxis = axis;
    midi = configByAxis[axis].midi;

    console.log("Setting axis", axis, midi.rootNote);
    osc = configByAxis[axis].osc;
    hid = configByAxis[axis].hid;
    aschema = schema[$type as T][axis];
    sensor = configByAxis[axis].sensor;
  }
  function setCategory(cat: string) {
    currentCat = cat;
  }
  const wifimodes = [
    { label: "Create Access Point", value: "AP" },
    { label: "Station (Connect to others)", value: "STA" },
  ];

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

<section class="buttonbar">
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
</section>
<h3>Quick settings</h3>

<QuickConfig bind:config />
<button class="primary Pause" on:click={pause} title="Pause sending data">
  {#if isPaused}
    Resume
  {/if}
  {#if !isPaused}
    Pause all output
  {/if}
</button>

<hr class="separator" />

<h3>Axis settings</h3>
{#if currentAxis}
  <div class="input">
    <h4>Editing axis :</h4>
    <div class="hero">
      <Select
        items={axisSelect}
        clearable={false}
        searchable={false}
        class="axis-select"
        value={currentAxis}
        --selected-item-color="var(--text-color)"
        --font-size="20px"
        --item-is-active-bg="var(--text-color)"
        --item-color="var(--text-color)"
        --item-bg="var(--bg-secondary)"
        --input-color="var(--text-color)"
        --item-hover-color="var(--text-color)"
        --item-hover-bg="var(--bg-lighter)"
        --border-radius="0"
        --border="0"
        --border-focused="0"
        --list-background="var(--bg-secondary)"
        --background="var(--bg-tertiary)"
        on:change={(evt) => setAxis(evt.detail.value)}
      />
    </div>
  </div>

  <AxisConfig {sensor} {aschema} {currentAxis} />
  <!-- <Checkbox label="Inverted" bind:value={sensorconf.inverted} /> -->

  <CategoryTab active={currentCat} onClick={(cat) => setCategory(cat)} />
  <section>
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
  <hr class="separator" />
  <Collapse title="OSC settings" bind:value={config.general.OSC_ENA}>
    <OscGlobalConfig
      bind:ip={config.general.OSC_IP}
      bind:port={config.general.OSC_PORT}
    />
  </Collapse>
  <hr class="separator" />
  <Collapse title="HID settings" bind:value={config.general.HidEnabled}
    ><HidGlobalConfig bind:mode={config.general.HidMode} /></Collapse
  >

  <hr class="separator" />
  <Collapse title="Board settings"
    ><BoardConfig bind:wifiMode={config.general.Wifi_mode} /></Collapse
  >
{/if}

<style>
  .board-settings {
    display: flex;
    flex-direction: column;
    justify-content: left;
  }
  /* :global(.axis-select .selected-item) {
    font-weight: bold;
    font-size: 27.2px;
    margin-block-start: 27.2px;
    margin-block-end: 27.2px;
  } */
  .buttonbar {
    display: flex;
    flex-direction: row-reverse;
    justify-content: space-between;
    margin-top: 2em;
    text-align: start;
    position: sticky;
    top: 5px;
    background-color: var(--bg-color);
    z-index: 100;
  }

  button:hover {
    background-color: var(--main-darker);
  }

  .separator {
    border: 0;
    height: 2px;
    background: var(--bg-lighter);
    margin: 20px 0;
  }

  /* .Download {
    background-color: rgba(106, 106, 106, 0.263);
  }
  .Download:hover {
    background-color: rgba(0.2, 0.1, 0.2, 0.3);
  } */

  .Pause {
    background-color: rgb(211, 211, 211);
  }
</style>
