<script lang="ts">
  import MenuButton from "./MenuButton.svelte";
  import Modal from "../modal.svelte";
  import { SettingsIcon } from "lucide-svelte";
  import InfoModal from "../InfoModal.svelte";
  import Text from "../form/Text.svelte";
  import { currentConfig } from "../../services";
    import Switch from "../form/Switch.svelte";
    import PillSwitch from "../form/PillSwitch.svelte";

  $: config = $currentConfig;
  $: mode = config?.general.MidiEnabled
    ? "midi"
    : config?.general.OSC_ENA
      ? "osc"
      : "osc";

  let open = true;

  function setMode(newMode: "osc" | "midi") {
    if (!config) return;
    config.general.MidiEnabled = newMode === "midi";
    config.general.OSC_ENA = newMode === "osc";
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
    </div>
  {/if}
</Modal>

<style scoped>
  .content {
    display: flex;
    flex-direction: column;
    gap: 16px;
    padding: 16px;
  }
  .row {
    display: flex;
    align-items: center;
    justify-content: space-between;
    width: 100%;
  }
  .left {
    display: flex;
    align-items: center;
    gap: 8px;
    font-size: 14px;
  }
  .pill-switch {
    border: 2px solid var(--main);
    position: relative;
    display: inline-flex;
    background-color: var(--bg-primary);
    border-radius: 20px;
    padding: 2px;
    gap: 2px;
  }

  .pill-indicator {
    position: absolute;
    top: 2px;
    left: 2px;
    height: calc(100% - 4px);
    width: calc(50% - 3px);
    background-color: var(--main);
    border-radius: 18px;
    transition: transform 0.3s cubic-bezier(0.4, 0, 0.2, 1);
    z-index: 0;
  }

  .pill-indicator.midi {
    transform: translateX(calc(100% + 2px));
  }

  .pill-switch input[type="radio"] {
    display: none;
  }

  .pill-switch label {
    position: relative;
    z-index: 1;
    padding: 0 10px;
    border-radius: 18px;
    cursor: pointer;
    transition: color 0.3s ease;
    background-color: transparent;
    color: var(--color);
    font-size: 14px;
    text-align: center;
    user-select: none;
    flex: 1;
  }

  .pill-switch label.active {
    color: var(--bg-primary);
  }

  .pill-switch label:hover {
    opacity: 0.8;
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
</style>
