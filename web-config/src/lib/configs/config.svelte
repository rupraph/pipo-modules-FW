<script lang="ts" generics="T extends PipoTypes">
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
  export let config: PipoConfig<T>;
  export let name: string;
  const dispatch = createEventDispatcher();
  let savingStatus = "none";
  const categories = [
    { value: "HID", label: "HID" },
    { value: "MIDI", label: "MIDI" },
    { value: "OSC", label: "OSC" },
  ];
  let configByAxis: ConfigByAxis<T>;
  let currentAxis: PipoKeys[T];
  let midi: MidiConfig;
  let osc: OscConfig;
  let hid: HidConfig;
  let sensor: SensorConfig;
  let aschema: AxisSchema;
  let axisSelect: { value: string; label: string }[] = [];
  let currentCat = "HID";
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

  function cal_offset(axis: PipoKeys[T]) {
    axios({
      method: "post",
      url: "/offsetcal",
      params: { axis },
    }).then(() => console.log("DONE"));
  }

  function pause() {
    axios.post("/pause").then(() => {
      console.log("Pausing...");
    });
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

  function reboot() {
    axios.get("/reboot").then(() => {
      console.log("Rebooting...");
    });
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
</script>

<article class="config">
  <section class="buttons">
    <button class="delete error" on:click={() => dispatch("delete")}
      >Delete</button
    >
    <!-- <button
      class="primary Download"
      on:click={download}
      title="Download the config file locally">Download config</button
    > -->
    <button class="primary Pause" on:click={pause} title="Pause sending data">
      &gt; / ||
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
  </section>
  {#if currentAxis}
    <Select
      items={axisSelect}
      clearable={false}
      value={currentAxis}
      --selected-item-color="var(--text-color)"
      --font-size="27.2px"
      --item-is-active-bg="var(--bg-tertiary)"
      --item-color="var(--text-color)"
      --item-bg="var(--bg-secondary)"
      --input-color="var(--text-color)"
      --item-hover-color="var(--text-color)"
      --item-hover-bg="var(--bg-lighter)"
      --border-radius="0"
      --border="0"
      --border-focused="0"
      --list-background="var(--bg-secondary)"
      --background="var(--bg-color)"
      on:change={(evt) => setAxis(evt.detail.value)}
    />
    <AxisConfig {sensor} {aschema} {currentAxis} />
    <!-- <Checkbox label="Inverted" bind:value={sensorconf.inverted} /> -->

    <CategoryTab
      items={categories}
      active={currentCat}
      onClick={(cat) => setCategory(cat)}
    />
    <section>
      {#if currentCat === "MIDI"}
        <MidiConfigForm {midi} />
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
        <OscConfigForm
          {osc}
          bind:oscEnabled={config.general.OSC_ENA}
          bind:ip={config.general.OSC_IP}
          bind:port={config.general.OSC_PORT}
        />
      {/if}
    </section>

    <Collapse title="Board Settings">
      <section class="board-settings">
        <!-- <button class="primary" on:click={switchwifimode} style="width: fit-content">{config.general.Wifi_mode}</button> -->
        <Select
          label="Wifi Mode"
          options={wifimodes}
          bind:value={config.general.Wifi_mode}
        />
        <button class="primary" on:click={reboot} style="width: fit-content"
          >Reboot</button
        >
      </section>
    </Collapse>

    <!-- {#if cat === "Touch"}
      <button class="primary" on:click={() => cal_offset(axis)}
        >Offset calib</button
      >
    {/if} -->
  {/if}
</article>

<style>
  .config {
    max-width: 100%;
  }
  .board-settings {
    display: flex;
    flex-direction: column;
    justify-content: left;
  }
  :global(.selected-item) {
    font-weight: bold;
    font-size: 27.2px;
    margin-block-start: 27.2px;
    margin-block-end: 27.2px;
  }
  .buttons {
    display: flex;
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
