<script lang="ts" generics="T extends PipoTypes">
  import { schema } from "../../schema";
  import { pipoType as type } from "../../services";
  import CCConfig from "./cc-config.svelte";
  import NoteConfig from "./note-config.svelte";
  import type {
    Axis,
    MidiConfig,
    SensorConfig,
    OscConfig,
    PipoConfig,
    PipoTypes,
    PipoKeys,
  } from "../../types";
  import Radio from "../form/Radio.svelte";
  import Checkbox from "../form/Checkbox.svelte";
  import axios from "axios";
  import Collapse from "../collapse.svelte";
  import Range from "../form/Range.svelte";
  import Text from "../form/Text.svelte";
  import Select from "../form/Select.svelte";
  export let config: PipoConfig<T>;
  const options = [
    { label: "Note", value: "1" },
    { label: "CC", value: "0" },
  ];

  function submit2() {
    axios({
      method: "post",
      url: "/save",
      params: { config: JSON.stringify(config) },
    }).then(() => console.log("DONE"));
  }

  async function submit() {
    const blob = new Blob([JSON.stringify(config)], {
      type: "application/json",
    });
    const formData = new FormData();
    formData.append("file", blob, "thisconfig.json"); //maybe we could pass the right name here

    await axios({
      method: "post",
      url: "/save",
      data: formData,
      headers: { "Content-Type": "multipart/form-data" },
    }).then(() => console.log("DONE"));
  }

  function test() {
    const blob = new Blob([JSON.stringify(config)], {
      type: "application/json",
    });
    const data = new FormData();
    data.append("config.json", blob);
    axios({
      method: "post",
      url: "/config",
      params: { config: JSON.stringify(config) },
    }).then(() => console.log("DONE"));
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
</script>

<article class="config">
  <Collapse title="Sensor settings">
    {#each getSensorConf() as [axis, sensorconf]}
      {@const { label, unit, min, max } = getSchema(axis)}
      <Collapse title={label}>
        <!-- <Checkbox label="Inverted" bind:value={sensorconf.inverted} /> -->
        <Range label="Deadzone" bind:value={sensorconf.deadzone} />
        <Range
          label={`limit min (${unit})`}
          bind:value={sensorconf.limit_min}
          {min}
          {max}
        />
        <Range
          label={`limit max (${unit})`}
          bind:value={sensorconf.limit_max}
          {min}
          {max}
        />
      </Collapse>
    {/each}
  </Collapse>
  <Collapse title="Data Output settings" closed>
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

  <section class="buttons">
    <div class="left-buttons">
      <!-- <button class="primary" on:click={test} title="Aplly the config without saving it">Set</button> -->
      <button
        class="primary"
        on:click={submit}
        title="Apply and save the config in pipo">Set & Save</button
      >
    </div>
    <div>
      <button
        class="primary Download"
        on:click={download}
        title="Download the config file locally">Download config</button
      >
    </div>
  </section>
</article>

<style>
  article {
    margin-left: 1em;
  }
  .config {
    max-width: min(600px, calc(100% - 80px));
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
  }

  .left-buttons {
    display: flex;
    gap: 1em;
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
