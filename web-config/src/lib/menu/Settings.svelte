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
  import { addToast } from "../toast";
  import { uiState } from "../ui-state";

  $: config = $currentConfig;
  $: mode = config?.general.MidiEnabled
    ? "midi"
    : config?.general.OSC_ENA
      ? "osc"
      : "osc";

  let open = false;
  let advancedMode = uiState.getAdvancedMode();

  $: {
    uiState.setAdvancedMode(advancedMode);
  }

  function setMode(newMode: "osc" | "midi") {
    if (!config) return;

    // Mutate the config object directly to avoid replacing the entire config
    config.general.MidiEnabled = newMode === "midi";
    config.general.OSC_ENA = newMode === "osc";
    // Disable BLE when switching to OSC mode
    if (newMode === "osc") {
      config.general.BLEEnabled = false;
    }
  }

  function filterPipoName(value: string): string {
    // Only allow alphanumeric characters
    return value.replace(/[^a-zA-Z0-9]/g, "");
  }

  function handlePipoNameInput(e: Event) {
    if (!config) return;
    const input = e.target as HTMLInputElement;
    let filtered = filterPipoName(input.value);

    // Apply max length limit
    if (filtered.length > schema.name.max) {
      filtered = filtered.slice(0, schema.name.max);
      input.value = filtered;
    }

    config.general.PipoName = filtered;
  }

  function filterIPv4(value: string): string {
    // Only allow numbers and dots for IPv4
    return value.replace(/[^0-9.]/g, "");
  }

  function handleIPInput(e: Event) {
    if (!config) return;
    const input = e.target as HTMLInputElement;
    let filtered = filterIPv4(input.value);

    // Apply max length limit for IPv4 (xxx.xxx.xxx.xxx = 15 chars)
    if (filtered.length > 15) {
      filtered = filtered.slice(0, 15);
      input.value = filtered;
    }

    config.general.OSC_IP = filtered;
  }

  function filterPort(value: string): string {
    // Only allow numbers for port
    return value.replace(/[^0-9]/g, "");
  }

  function handlePortInput(e: Event) {
    if (!config) return;
    const input = e.target as HTMLInputElement;
    let filtered = filterPort(input.value);

    // Apply max length limit for port (65535 = 5 digits)
    if (filtered.length > 5) {
      filtered = filtered.slice(0, 5);
      input.value = filtered;
    }

    config.general.OSC_PORT = parseInt(filtered) || 0;
  }

  function reboot() {
    addToast({
      type: "error",
      message:
        "Pipo is rebooting... Please wait a few seconds and reload the page. Make sure WiFi is reconnected.",
      timeout: 8000,
    });
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
        <span class="label">Ouput mode</span>
        <InfoModal>
          <p>
            Select the general output mode of the Pipo: MIDI or OSC. MIDI can
            only be sent over USB or BLE, OSC can only be sent over Wifi.
          </p>
          <p>This requires reboot after saving.</p>
        </InfoModal>
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
          <span class="label">Destination IP</span>
          <InfoModal>
            <p>
              This is the IP address of the destination PC where OSC data will
              be sent. The IP is subject to change when you switch network, make
              sure this is correct.
            </p>
          </InfoModal>
          <input
            type="text"
            bind:value={config.general.OSC_IP}
            on:input={handleIPInput}
          />
        </div>

        <div class="row">
          <span class="label">Port</span>
          <InfoModal>
            <p>
              This is the port number where the OSC data is sent. Make sure the
              receiving device listens on this port, and that the port is open
              and not blocked by any firewall settings.
            </p>
          </InfoModal>
          <input
            type="text"
            bind:value={config.general.OSC_PORT}
            on:input={handlePortInput}
          />
        </div>
        <div class="row">
          <span class="label">Send Battery level</span>
          <InfoModal>
            <p>
              This option enables the battery level to be sent over OSC at
              regular intervals.
            </p>
          </InfoModal>
          <PillSwitch label="" bind:value={config.general.OSC_Batt} />
        </div>
      {:else}
        <div class="row">
          <span class="label">Enable BLE</span>
          <InfoModal>
            <p>
              This enables sending Midi over BLE (Bluetooth Low Energy). When
              toggled, this requires a reboot after saving the change. The BLE
              connection process depends on your OS:
            </p>
            <ul
              style="justify-content: flex-start; text-align: left; margin-top: 0.5em;"
            >
              <li>
                On MacOS, this is native and done through the native "Audio MIDI
                Setup" panel
              </li>
              <li>
                On mobiles this is done directly in the receiving music app
              </li>
              <li>On windows this requires a third party bridge app</li>
            </ul>
          </InfoModal>
          <PillSwitch label="" bind:value={config.general.BLEEnabled} />
        </div>
      {/if}
      <div class="row">
        <span class="label">Pipo Name</span>
        <InfoModal>
          <p>
            The Pipo Name is used to identify your device. It is used for the
            WiFi name, access URL (pipo-xxx.local), and OSC address prefix.
            Choose a unique name without spaces or special characters. Please
            power cycle after saving the changes.
          </p>
        </InfoModal>
        <div class="text-input-wrapper">
          <input
            type="text"
            bind:value={config.general.PipoName}
            on:input={handlePipoNameInput}
          />
        </div>
      </div>
      <div class="row">
        <span class="label">Advanced Mode</span>
        <InfoModal>
          <p>
            When enabled, additional advanced settings will be displayed
            throughout the configuration interface.
          </p>
        </InfoModal>
        <PillSwitch label="" bind:value={advancedMode} />
      </div>
      <div class="row">
        <span class="label">
          <a
            href="https://pipointerfaces.com/manual"
            target="_blank"
            rel="noopener noreferrer"
            class="manual-button"
          >
            Online Manual link
          </a>
        </span>
        <div></div>
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

  .row {
    display: grid;
    grid-template-columns: 40% 32px 1fr;
    align-items: center;
    gap: 12px;
  }

  .row > :nth-child(2) {
    justify-self: center;
  }

  .row > :nth-child(3) {
    justify-self: end;
  }

  .label {
    text-align: left;
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

  .text-input-wrapper {
    max-width: 200px;
    width: 100%;
    justify-self: end;
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
