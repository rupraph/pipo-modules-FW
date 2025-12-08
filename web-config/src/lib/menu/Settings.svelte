<script lang="ts">
  import MenuButton from "./MenuButton.svelte";
  import Modal from "../modal.svelte";
  import { SettingsIcon } from "lucide-svelte";
  import InfoModal from "../InfoModal.svelte";
  import Text from "../form/Text.svelte";
  import { currentConfig } from "../../services";
  import Switch from "../form/Switch.svelte";
  import PillSwitch from "../form/PillSwitch.svelte";
  import { schema } from "../../schema";
  import { pipoio } from "../../pipoio";

  $: config = $currentConfig;
  $: mode = config?.general.MidiEnabled
    ? "midi"
    : config?.general.OSC_ENA
      ? "osc"
      : "osc";

  let open = false;

  function setMode(newMode: "osc" | "midi") {
    if (!config) return;
    config.general.MidiEnabled = newMode === "midi";
    config.general.OSC_ENA = newMode === "osc";
    currentConfig.set(config);
  }

  const validate = (name) => {
    // firbid spaces
    if (name.includes(" ")) {
      return "Spaces are not allowed";
    }
    // forbid any other character than a-zA-Z0-9
    const regex = /^[a-zA-Z0-9]+$/;
    if (!regex.test(name)) {
      return "Only letters and numbers are allowed";
    }
    return "";
  };

  function reboot() {
    pipoio.get("/reboot").then(() => console.log("Rebooting..."));
  }
</script>

<MenuButton on:click={() => (open = !open)}>
  <SettingsIcon color="var(--grey)" />
</MenuButton>

<Modal bind:open>
  {#if config}
    <div class="content">
      <div class="row">
        <div class="left">
          Ouput mode <InfoModal>
            <p>
              Select the output mode for your device. Different modes may offer
              various functionalities and performance characteristics.
            </p>
          </InfoModal>
        </div>
        <div class="pill-switch">
          <div class="pill-indicator" class:midi={mode === "midi"}></div>
          <input
            type="radio"
            name="output-mode"
            value="osc"
            id="osc"
            checked={mode === "osc"}
            on:change={() => setMode("osc")}
          />
          <input
            type="radio"
            name="output-mode"
            value="midi"
            id="midi"
            checked={mode === "midi"}
            on:change={() => setMode("midi")}
          />
          <label
            for="osc"
            on:click={() => setMode("osc")}
            class:active={mode === "osc"}>OSC</label
          >
          <label
            for="midi"
            on:click={() => setMode("midi")}
            class:active={mode === "midi"}>MIDI</label
          >
        </div>
      </div>
      {#if mode === "osc"}
        <div class="row">
          <div class="left">
            Destination IP
            <InfoModal>
              <p>
                Set the destination IP address for sending OSC or MIDI messages.
                Ensure that the IP address is correct to establish a successful
                connection.
              </p>
            </InfoModal>
          </div>
          <input type="text" bind:value={config.general.OSC_IP} />
        </div>

        <div class="row">
          <div class="left">
            Port
            <InfoModal>
              <p>
                Specify the port number used for communication. Make sure the
                port is open and not blocked by any firewall settings.
              </p>
            </InfoModal>
          </div>
          <input type="text" bind:value={config.general.OSC_PORT} />
        </div>
      {:else}
        <div class="row">
          <PillSwitch
            label="Enable BLE"
            bind:value={config.general.BLEEnabled}
          />
        </div>
      {/if}
      <div class="row">
        <div class="left">
          Pipo Name
          <InfoModal>
            <p>
              The Pipo Name is used to identify your device on the network and
              when connecting via Bluetooth. Choose a unique name without spaces
              or special characters.
            </p>
          </InfoModal>
        </div>
        <div style="width: 200px;">
          <Text
            label=""
            bind:value={config.general.PipoName}
            maxlength={schema.name.max}
            minlength={schema.name.min}
            {validate}
          />
        </div>
      </div>
      <div class="row">
        <a
          href="https://pipointerfaces.com/manual"
          target="_blank"
          rel="noopener noreferrer"
          class="manual-button"
        >
          Online Manual link
        </a>
        <button class="primary" on:click={reboot} style="width: fit-content"
          >Reboot</button
        >
      </div>
    </div>
  {/if}
</Modal>

<style scoped>
  .content {
    display: flex;
    flex-direction: column;
    gap: 16px;
    padding: 16px;
    box-sizing: border-box;
  }

  /* Pill Switch - Component Specific */
  .pill-indicator.midi {
    transform: translateX(calc(100% + 2px));
  }

  input[type="text"] {
    background-color: var(--bg-secondary);
    border: 1px solid var(--main);
    border-radius: 4px;
    height: 24px;
    padding: 0;
    color: var(--text-color);
    font-size: 14px;
    width: 150px;
  }

  /* .content :global(a.manual-button) {
    display: inline-flex;
    align-items: center;
    justify-content: center;
    width: fit-content;
    padding: 8px 16px;
    background-color: var(--bg-secondary);
    color: var(--main);
    border: 1px solid var(--main);
    border-radius: 4px;
    text-decoration: none;
    font-size: 14px;
    font-weight: 500;
    cursor: pointer;
    transition: background-color 0.2s;
  }

  .content :global(a.manual-button:hover) {
    background-color: var(--main);
    color: var(--bg-secondary);
  } */
</style>
