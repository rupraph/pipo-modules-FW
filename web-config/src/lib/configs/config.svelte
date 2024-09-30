<script lang="ts" generics="T extends PipoTypes">
  import { configSave } from "../../services/config";

  import { pipoio } from "../../pipoio";
  import { createEventDispatcher, onMount } from "svelte";
  import MinMax from "../form/MinMax.svelte";
  import { schema } from "../../schema";
  import { pipoType as type } from "../../services";
  import CCConfig from "./cc-config.svelte";
  import NoteConfig from "./note-config.svelte";
  import {
    type MidiConfig,
    type SensorConfig,
    type OscConfig,
    type PipoConfig,
    type PipoTypes,
    type SensorValues,
    type PipoKeys,
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
  const sensorValues: SensorValues<T> = {};
  const withinWindowValues: SensorValues<T> = {};
  pipoio.on("sensor", ({ axis, value, withinWindow }) => {
    const now = Date.now();
    if (!sensorValues[axis]) {
      sensorValues[axis] = {
        value: 0,
        timestamp: now,
      };
    }
    sensorValues[axis] = value;
    withinWindowValues[axis] = withinWindow;
  });
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

  function getOscConf() {
    return Object.entries(config.engine["engine-osc"]) as unknown as [
      PipoKeys[T],
      OscConfig,
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
  onMount(() => {
    setInterval(() => configSave.update({ ...config }), 2000);
  });
</script>

<article class="config">
  <section class="buttons">
    <button class="delete error" on:click={() => dispatch("delete")}
      >Delete</button
    >
    <button
      class="primary Download"
      on:click={download}
      title="Download the config file locally">Download config</button
    >
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
    {#each getSensorConf() as [axis, sensorconf]}
      {@const { label, unit, min, max, step } = getSchema(axis)}
      <Collapse title={label} open>
        <!-- <Checkbox label="Inverted" bind:value={sensorconf.inverted} /> -->

        <Checkbox label="Threshold mode" bind:value={sensorconf.mode} />
        {#if sensorconf.mode === true}
          <Checkbox
            label="Window threshold"
            bind:value={sensorconf.threshold_mode}
          />
        {/if}
        <MinMax
          label="Sensor Range"
          bind:low={sensorconf.limit_min}
          bind:high={sensorconf.limit_max}
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
        <!-- <Range label="Deadzone" bind:value={sensorconf.deadzone} /> -->
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
              value={midiconfig.translator_mode}
              on:change={(evt) => {
                midiconfig.translator_mode = evt.detail;
              }}
            />
            {#if midiconfig.translator_mode === 0}
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
              <Range label="OSC Min" bind:value={oscconf.output_min} />
              <Range label="OSC Max" bind:value={oscconf.output_max} />
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

  .Download {
    background-color: rgba(106, 106, 106, 0.263);
  }
  .Download:hover {
    background-color: rgba(0.2, 0.1, 0.2, 0.3);
  }
</style>
