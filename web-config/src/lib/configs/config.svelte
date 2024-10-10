<script lang="ts" generics="T extends PipoTypes">
  import { configSave } from "../../services/config";
  import { pipoio } from "../../pipoio";
  import { createEventDispatcher, onMount } from "svelte";
  import MinMax from "../form/MinMax.svelte";
  import { schema } from "../../schema";
  import { pipoType as type } from "../../services";
  import CCConfig from "./cc-config.svelte";
  import NoteConfig from "./note-config.svelte";
  import Select from "svelte-select";
  import {
    type SensorConfig,
    type PipoConfig,
    type PipoTypes,
    type SensorValues,
    type PipoKeys,
    type SmoothSensorValues,
    isHisteresisMode,
    isContinuousMode,
    type ConfigByAxis,
    type AxisConfig,
    type AxisSchema,
    type MidiConfig,
    type OscConfig,
    type HidConfig,
  } from "../../types";
  import Radio from "../form/Radio.svelte";
  import Checkbox from "../form/Checkbox.svelte";
  import axios from "axios";
  import Collapse from "../collapse.svelte";
  import Range from "../form/Range.svelte";
  import Text from "../form/Text.svelte";
  import LoadingButton from "../form/LoadingButton.svelte";
  import CategoryTab from "./category-tab.svelte";
  export let config: PipoConfig<T>;
  export let name: string;
  const dispatch = createEventDispatcher();
  let savingStatus = "none";
  const smoothValues: SmoothSensorValues<T> = {};
  const withinWindowValues: SensorValues<T> = {};
  const sensorValues: SensorValues<T> = {};

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

  pipoio.on("sensor", ({ axis, value, withinWindow }) => {
    withinWindowValues[axis] = withinWindow;
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
    Object.entries(smoothValues).forEach(([axis, value]) => {
      if (value.dt > 0) {
        sensorValues[axis] =
          value.old + (value.new - value.old) * (value.dt / 1000);
      }
    });
    requestAnimationFrame(animateSensor);
  }
  animateSensor();
  const options = [
    { label: "Note", value: "1" },
    { label: "CC", value: "0" },
  ];

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
    <!-- <Checkbox label="Inverted" bind:value={sensorconf.inverted} /> -->
    {#if aschema.cat !== "Touch"}
      <Checkbox label="Threshold mode" bind:value={sensor.mode} />
      {#if sensor.mode === true}
        <Checkbox label="Window threshold" bind:value={sensor.th_mode} />
      {/if}
    {/if}
    <MinMax
      label="Sensor Range"
      bind:low={sensor.lmin}
      bind:high={sensor.lmax}
      value={sensorValues[currentAxis]}
      mode={isContinuousMode(sensor) || isHisteresisMode(sensor)
        ? "double"
        : "single"}
      cursorActive={withinWindowValues[currentAxis]}
      min={aschema.min}
      max={aschema.max}
      step={aschema.step}
      minLabel={`min (${aschema.unit})`}
      maxLabel={`max (${aschema.unit})`}
    />
    <CategoryTab
      items={categories}
      active={currentCat}
      onClick={(cat) => setCategory(cat)}
    />
    {#if currentCat === "MIDI"}
      <section>
        <!-- <Checkbox label="enabled" bind:value={midiconfig.enabled} /> -->
        <Range
          label="Midi Channel"
          bind:value={midi.channel}
          min={1}
          max={16}
        />
        <Radio
          label="Message Type"
          {options}
          value={midi.tl_mode}
          on:change={(evt) => {
            midi.tl_mode = evt.detail;
          }}
        />
        {#if midi.tl_mode === 0}
          <CCConfig config={midi} />
        {:else}
          <NoteConfig config={midi} />
        {/if}
      </section>
    {/if}
    {#if currentCat === "HID"}
      <section>
        <h4>Keyboard/Mouse mode settings</h4>
        <Checkbox label="HID Enabled" bind:value={config.general.HidEnabled} />
        <Select
          label="HID Mode"
          options={[
            { label: "Keyboard", value: 2 },
            { label: "Mouse", value: 1 },
          ]}
          bind:value={config.general.HidMode}
        />
        <h4>Please restart Pipo after enabling or switching HID mode</h4>
        <h4>
          NOTE: The available mapping options below will depend on the sensor
          and Hid mode
        </h4>
        {#if sensor.mode === true && config.general.HidMode === 2}
          <h4>
            Map a keyboard key. Address format for "u" would be: "KEY_u" (or
            KEY_UP,KEY_ENTER,...)
          </h4>
          <Checkbox label="Stroke continuous" bind:value={hid.stroke_mode} />
          <Text label="Address" bind:value={hid.addr} />
          {#if hid.stroke_mode && sensor.th_mode === true}
            <Text label="Address2" bind:value={hid.addr2} />
          {/if}
        {:else if sensor.mode === true && config.general.HidMode === 1}
          <h4>Map a mouse button ("LEFT" or "RIGHT")</h4>
          <Checkbox label="Stroke continuous" bind:value={hid.stroke_mode} />
          <Text label="Address" bind:value={hid.addr} />
        {:else if sensor.mode === false && config.general.HidMode === 2}
          <h4>
            Not possible to map a continuous sensor axis to a key stoke (must
            change to Threshold mode)
          </h4>
        {:else if sensor.mode === false && config.general.HidMode === 1}
          <h4>
            Map a continuous sensor axis to a mouse axis (Address can be
            "X","Y","WHEEL","PAN")
          </h4>
          <Text label="Address" bind:value={hid.addr} />
        {:else}
          PROBLEM !
        {/if}
      </section>
    {/if}
    {#if currentCat === "OSC"}
      <section>
        <h4>OSC Network settings</h4>
        <Checkbox label="OSC Enabled" bind:value={config.general.OSC_ENA} />
        <Text label="OSC IP" bind:value={config.general.OSC_IP} />
        <Range label="OSC Port" bind:value={config.general.OSC_PORT} />
        <section>
          <!-- <Checkbox label="Enabled" bind:value={oscconf.enabled} /> -->
          <Checkbox label="Mode_raw" bind:value={osc.mode_raw} />
          {#if !osc.mode_raw}
            <Range label="OSC Min" bind:value={osc.osc_min} />
            <Range label="OSC Max" bind:value={osc.osc_max} />
          {/if}
        </section>
      </section>
    {/if}
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
