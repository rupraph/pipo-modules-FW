<script lang="ts">
  import MenuButton from "./MenuButton.svelte";
  import Modal from "../modal.svelte";
  import { SettingsIcon } from "lucide-svelte";
  import InfoModal from "../InfoModal.svelte";
  import Text from "../form/Text.svelte";
  import {
    currentConfig,
    settingsModalOpen,
    hasUnsavedChanges,
    modeWillChange,
    pipoNameWillChange,
    bleWillChange,
    pipoType,
    pipoInfo,
  } from "../../services";
  import { getDefaultDeadband } from "../../defaults";
  import Switch from "../form/Switch.svelte";
  import NumberInput from "../form/Number.svelte";
  import type { WifiPowerKey } from "../../types";
  import PillSwitch from "../form/PillSwitch.svelte";
  import { schema } from "../../schema";
  import { pipoio } from "../../pipoio";
  import { addToast } from "../toast";
  import { uiState } from "../ui-state";
  import { saveConfig, savingStatus } from "../../services/config-saver";

  $: config = $currentConfig;
  $: mode = config?.general.MidiEnabled
    ? "midi"
    : config?.general.OSC_ENA
      ? "osc"
      : "osc";

  let open = false;

  // Sync modal state with store
  $: settingsModalOpen.set(open);
  function setMode(newMode: "osc" | "midi") {
    if (!config) return;

    // Mutate the config object directly to avoid replacing the entire config
    config.general.MidiEnabled = newMode === "midi";
    config.general.OSC_ENA = newMode === "osc";
    // Disable BLE when switching to OSC mode
    if (newMode === "osc") {
      config.general.BLEEnabled = false;
    }
    // Restore default deadbands when switching to MIDI (filter is OSC-only UI)
    if (newMode === "midi") {
      const type = $pipoType;
      for (const channel of Object.keys(config.inputs)) {
        config.inputs[channel].deadband = getDefaultDeadband(type, channel);
      }
      config.general.BLEEnabled = true;
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

  const wifiPowerKeys = Object.keys(schema.wifiPower) as WifiPowerKey[];

  // Older configs lack the power-save keys: fill defaults when enabling
  $: if (config?.general.Wifi_ps_ena) {
    for (const key of wifiPowerKeys) {
      if (config.general[key] === undefined) {
        config.general[key] = schema.wifiPower[key].default;
      }
    }
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
            Select the general data output mode of the Pipo: MIDI or OSC.<br />
            <br />
            MIDI: sends Midi data as a USB controller, or wirelessly using BLE (Bluetooth
            Low Energy). Midi is easier to setup with music software but has a lower
            resolution format. Also, when using BLE, connection must be manually
            established each time.
            <br />
            <br />
            OSC: sends data wirelessly over Wifi. It allows for connecting multiple
            devices, use high resolution data, and is more flexible and convenient
            for advanced users (allows for auto-reconnection, device swapping, etc...).
            However, initial setup requires some network configuration, which is
            less straightforward than MIDI.
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
            WiFi name, config access URL (pipo-xxx.local), and OSC address
            prefix.<br /><br />
            Choose a unique name without spaces or special characters. Please power
            cycle after saving the change.<br /><br />
            Note: After changing the name, your computer might take a while to display
            the new wifi name due to caching (you can try enabling/disabling your
            PC wifi to force a refresh, or search in "other networks"). When connected
            again, make sure to use the new address to access this configuration
            page.
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
        <span class="label">Lock buttons</span>
        <InfoModal>
          <p>This option disables all physical buttons on the device</p>
        </InfoModal>
        <PillSwitch label="" bind:value={config.general.Button_disa} />
      </div>
      <div class="row">
        <span class="label">WiFi battery saving</span>
        <InfoModal>
          <p>
            Turns the Pipo WiFi access point off when it is not needed, so the
            radio can sleep and save battery.<br /><br />
            The access point is on for a while after boot, and as long as this
            page is open on screen. Once the page is closed or hidden (screen
            locked, other app) for the set time, the access point stops, and a
            phone still connected to it is disconnected.<br /><br />
            If the Pipo is connected to a known WiFi network, this page stays
            reachable there at pipo-{config.general.PipoName}.local. In OSC mode,
            the access point stays on if there is no known network, or if the
            OSC target is a device on the access point. In MIDI mode without a
            known network, WiFi turns off completely (BLE MIDI keeps working).<br
            /><br />
            Hold the play/pause button to turn the access point back on. If
            buttons are locked, reboot the Pipo instead.
          </p>
        </InfoModal>
        <PillSwitch label="" bind:value={config.general.Wifi_ps_ena} />
      </div>
      {#if config.general.Wifi_ps_ena}
        <div class="row">
          <span class="label">AP time after boot (s)</span>
          <div></div>
          <NumberInput
            label=""
            min={schema.wifiPower.Wifi_ap_boot_s.min}
            max={schema.wifiPower.Wifi_ap_boot_s.max}
            step={schema.wifiPower.Wifi_ap_boot_s.step}
            bind:value={config.general.Wifi_ap_boot_s}
          />
        </div>
        <div class="row">
          <span class="label">AP time after last page use (s)</span>
          <div></div>
          <NumberInput
            label=""
            min={schema.wifiPower.Wifi_ap_idle_s.min}
            max={schema.wifiPower.Wifi_ap_idle_s.max}
            step={schema.wifiPower.Wifi_ap_idle_s.step}
            bind:value={config.general.Wifi_ap_idle_s}
          />
        </div>
        <div class="row">
          <span class="label">WiFi lost, AP back after (s)</span>
          <div></div>
          <NumberInput
            label=""
            min={schema.wifiPower.Wifi_sta_lost_s.min}
            max={schema.wifiPower.Wifi_sta_lost_s.max}
            step={schema.wifiPower.Wifi_sta_lost_s.step}
            bind:value={config.general.Wifi_sta_lost_s}
          />
        </div>
        <div class="row">
          <span class="label">Button hold to open AP (ms)</span>
          <div></div>
          <NumberInput
            label=""
            min={schema.wifiPower.Wifi_ap_btn_ms.min}
            max={schema.wifiPower.Wifi_ap_btn_ms.max}
            step={schema.wifiPower.Wifi_ap_btn_ms.step}
            bind:value={config.general.Wifi_ap_btn_ms}
          />
        </div>
      {/if}

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
      <div class="info-section">
        <div class="info-col">
          <span class="info-label">Firmware</span>
          <span class="info-value">{$pipoInfo?.version ?? "—"}</span>
        </div>
        <div class="info-col">
          <span class="info-label">Board Rev</span>
          <span class="info-value"
            >{$pipoInfo?.hw_rev
              ? $pipoInfo.hw_rev[0] + "." + $pipoInfo.hw_rev[1]
              : "—"}</span
          >
        </div>
        <div class="info-col">
          <span class="info-label">Battery type</span>
          <span class="info-value">{$pipoInfo?.batt_type ?? "—"}</span>
        </div>
      </div>

      <!-- Save Button (shown when there are unsaved changes) -->
      {#if $hasUnsavedChanges}
        <div class="save-button-container">
          <button
            class="save-btn"
            class:loading={$savingStatus === "loading"}
            class:success={$savingStatus === "success"}
            class:error={$savingStatus === "error"}
            on:click={() => config && saveConfig(config)}
            disabled={$savingStatus === "loading"}
          >
            {#if $savingStatus === "loading"}
              <span class="spinner"></span>
              Saving...
            {:else if $savingStatus === "success"}
              ✓ Saved!
            {:else if $savingStatus === "error"}
              ✗ Error
            {:else if $modeWillChange || $pipoNameWillChange || $bleWillChange}
              Save and Reboot
            {:else}
              Save Changes
            {/if}
          </button>
        </div>
      {/if}
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

  /* Save Button Styles */
  .save-button-container {
    /* Stays visible at the bottom of the scrolling modal */
    position: sticky;
    bottom: -16px; /* cancel .content bottom padding */
    margin-top: 8px;
    padding: 16px 0;
    border-top: 1px solid var(--bg-secondary);
    background-color: var(--bg-network);
    display: flex;
    justify-content: center;
  }

  .save-btn {
    display: flex;
    align-items: center;
    gap: 8px;
    padding: 12px 24px;
    background-color: var(--main);
    color: var(--bg-primary);
    border: none;
    border-radius: 20px;
    font-size: 15px;
    font-weight: 600;
    cursor: pointer;
    box-shadow: 0 2px 8px rgba(0, 0, 0, 0.2);
    transition: all 0.2s ease;
    min-width: 140px;
    justify-content: center;
  }

  .save-btn:hover:not(:disabled) {
    transform: translateY(-1px);
    box-shadow: 0 4px 12px rgba(0, 0, 0, 0.3);
    background-color: var(--main-lighter);
  }

  .save-btn:active:not(:disabled) {
    transform: translateY(0);
  }

  .save-btn:disabled {
    cursor: wait;
    opacity: 0.8;
  }

  .save-btn.success {
    background-color: var(--green);
    color: white;
  }

  .save-btn.error {
    background-color: var(--red);
    color: white;
  }

  .spinner {
    width: 14px;
    height: 14px;
    border: 2px solid var(--bg-primary);
    border-bottom-color: transparent;
    border-radius: 50%;
    display: inline-block;
    box-sizing: border-box;
    animation: rotation 1s linear infinite;
  }

  @keyframes rotation {
    0% {
      transform: rotate(0deg);
    }
    100% {
      transform: rotate(360deg);
    }
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

  .info-section {
    display: flex;
    flex-direction: row;
    justify-content: space-between;
    gap: 6px;
    /* padding: 8px; */
    /* background-color: var(--bg-secondary); */
    border-radius: 8px;
  }

  .info-col {
    display: flex;
    /* justify-content: space-between; */
    gap: 4px;
    align-items: center;
  }

  .info-label {
    font-size: 10px;
    color: var(--text-secondary, #888);
  }

  .info-value {
    font-size: 10px;
    font-weight: 600;
    color: var(--text-color);
  }
</style>
