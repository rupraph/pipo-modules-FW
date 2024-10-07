<script lang="ts" generics="T extends PipoTypes">
  import { configSave } from "../../services/config";

  import { pipoio } from "../../pipoio";
  import { createEventDispatcher, onDestroy, onMount } from "svelte";
  import MinMax from "../form/MinMax.svelte";
  import { schema } from "../../schema";
  import { pipoType as type } from "../../services";
  import CCConfig from "./cc-config.svelte";
  import NoteConfig from "./note-config.svelte";
  import {
    type MidiConfig,
    type SensorConfig,
    type OscConfig,
    type HidConfig,
    type PipoConfig,
    type PipoTypes,
    type SensorValues,
    type PipoKeys,
    type SmoothSensorValues,
    type SmoothSensorValue,
    isHisteresisMode,
    isContinuousMode,
  } from "../../types";
  import Radio from "../form/Radio.svelte";
  import Checkbox from "../form/Checkbox.svelte";
  import axios from "axios";
  import Collapse from "../collapse.svelte";
  import Range from "../form/Range.svelte";
  import Text from "../form/Text.svelte";
  import Select from "../form/Select.svelte";
  import LoadingButton from "../form/LoadingButton.svelte";
  export let config: PipoConfig<T>;
  export let name: string;
  const dispatch = createEventDispatcher();
  let savingStatus = "none";
  const smoothValues: SmoothSensorValues<T> = {};
  const withinWindowValues: SensorValues<T> = {};
  const sensorValues: SensorValues<T> = {};

  let sensorConfs = [];
  let groupedSensorConfs = {};

  onMount(() => {
    sensorConfs = getSensorConf();
    groupedSensorConfs = groupBySection(sensorConfs);
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

  function getMidiConfigs() {
    return Object.entries(config.engine["engine-midi"]) as unknown as [
      PipoKeys[T],
      MidiConfig,
    ][];
  }

  function switchwifimode() {
    axios.post("/wifimode").then(() => {
      console.log("Switching wifi mode...");
    });
  }

  function getSensorConf() {
    return Object.entries(config.sensor) as unknown as [
      PipoKeys[T],
      SensorConfig,
    ][];
  }

  function groupBySection(sensorConfs: [PipoKeys[T], SensorConfig][]) {
    const sections = {};
    for (const [axis, sensorconf] of sensorConfs) {
      const { cat } = getSchema(axis);
      console.log(cat);
      if (!sections[cat]) {
        sections[cat] = [];
      }
      sections[cat].push([axis, sensorconf]);
    }
    console.log(sections);
    return sections;
  }

  function getOscConf() {
    return Object.entries(config.engine["engine-osc"]) as unknown as [
      PipoKeys[T],
      OscConfig,
    ][];
  }

  function getHidConf() {
    return Object.entries(config.engine["engine-hid"]) as unknown as [
      PipoKeys[T],
      HidConfig,
    ][];
  }

  function getSchema(axis: PipoKeys[T]) {
    return schema[$type as T][axis];
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
  <Collapse title="Sensor settings" open>
    {#each Object.entries(groupedSensorConfs) as [cat, sensors]}
      <Collapse title={cat}>
        {#each sensors as [axis, sensorconf]}
          {@const { label, cat, unit, min, max, step } = getSchema(axis)}
          <Collapse title={label}>
            <!-- <Checkbox label="Inverted" bind:value={sensorconf.inverted} /> -->
            {#if cat !== "Touch"}
              <Checkbox label="Threshold mode" bind:value={sensorconf.mode} />
              {#if sensorconf.mode === true}
                <Checkbox
                  label="Window threshold"
                  bind:value={sensorconf.th_mode}
                />
              {/if}
            {/if}
            <MinMax
              label="Sensor Range"
              bind:low={sensorconf.lmin}
              bind:high={sensorconf.lmax}
              value={sensorValues[axis]}
              mode={isContinuousMode(sensorconf) || isHisteresisMode(sensorconf)
                ? "double"
                : "single"}
              cursorActive={withinWindowValues[axis]}
              {min}
              {max}
              {step}
              minLabel={`min (${unit})`}
              maxLabel={`max (${unit})`}
            />
            {#if cat === "Touch"}
              <button class="primary" on:click={() => cal_offset(axis)}
                >Offset calib</button
              >
            {/if}
          </Collapse>
        {/each}
      </Collapse>
    {/each}
  </Collapse>
  <Collapse title="Data Output settings">
    <Collapse title="Midi Output">
      {#each getMidiConfigs() as [axis, midiconfig]}
        {@const { label } = getSchema(axis)}
        <section>
          <Collapse title={label} bind:value={midiconfig.enabled}>
            <!-- <Checkbox label="enabled" bind:value={midiconfig.enabled} /> -->
            <Range
              label="Midi Channel"
              bind:value={midiconfig.channel}
              min={1}
              max={16}
            />
            <Radio
              label="Message Type"
              {options}
              value={midiconfig.tl_mode}
              on:change={(evt) => {
                midiconfig.tl_mode = evt.detail;
              }}
            />
            {#if midiconfig.tl_mode === 0}
              <CCConfig config={midiconfig} />
            {:else}
              <NoteConfig config={midiconfig} />
            {/if}
          </Collapse>
        </section>
      {/each}
    </Collapse>
    <Collapse title="OSC output">
      <h4>OSC Network settings</h4>
      <Checkbox label="OSC Enabled" bind:value={config.general.OSC_ENA} />
      <Text label="OSC IP" bind:value={config.general.OSC_IP} />
      <Range label="OSC Port" bind:value={config.general.OSC_PORT} />
      {#each getOscConf() as [axis, oscconf]}
        {@const { label } = getSchema(axis)}
        <section>
          <Collapse title={label} bind:value={oscconf.enabled}>
            <!-- <Checkbox label="Enabled" bind:value={oscconf.enabled} /> -->
            <Checkbox label="Mode_raw" bind:value={oscconf.mode_raw} />
            {#if !oscconf.mode_raw}
              <Range label="OSC Min" bind:value={oscconf.osc_min} />
              <Range label="OSC Max" bind:value={oscconf.osc_max} />
            {/if}
          </Collapse>
        </section>
      {/each}
    </Collapse>
    <Collapse title="HID output">
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
        NOTE: The available mapping options below will depend on the sensor and
        Hid mode
      </h4>
      {#each getHidConf() as [axis, hidconf]}
        {@const { label } = getSchema(axis)}
        <section>
          <Collapse title={label} bind:value={hidconf.enabled}>
            {#if config.sensor[axis].mode === true && config.general.HidMode === 2}
              <h4>
                Map a keyboard key. Address format for "u" would be: "KEY_u" (or
                KEY_UP,KEY_ENTER,...)
              </h4>
              <Checkbox
                label="Stroke continuous"
                bind:value={hidconf.stroke_mode}
              />
              <Text label="Address" bind:value={hidconf.addr} />
              {#if hidconf.stroke_mode && config.sensor[axis].th_mode === true}
                <Text label="Address2" bind:value={hidconf.addr2} />
              {/if}
            {:else if config.sensor[axis].mode === true && config.general.HidMode === 1}
              <h4>Map a mouse button ("LEFT" or "RIGHT")</h4>
              <Checkbox
                label="Stroke continuous"
                bind:value={hidconf.stroke_mode}
              />
              <Text label="Address" bind:value={hidconf.addr} />
            {:else if config.sensor[axis].mode === false && config.general.HidMode === 2}
              <h4>
                Not possible to map a continuous sensor axis to a key stoke
                (must change to Threshold mode)
              </h4>
            {:else if config.sensor[axis].mode === false && config.general.HidMode === 1}
              <h4>
                Map a continuous sensor axis to a mouse axis (Address can be
                "X","Y","WHEEL","PAN")
              </h4>
              <Text label="Address" bind:value={hidconf.addr} />
            {/if}
          </Collapse>
        </section>
      {/each}
    </Collapse>
  </Collapse>
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
