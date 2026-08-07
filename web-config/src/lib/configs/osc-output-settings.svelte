<script lang="ts">
  import type { OscConfig } from "../../types";
  import { currentConfig, pipoType } from "../../services/config";
  import { uiState } from "../ui-state/store";
  import Number from "../form/Number.svelte";
  import InfoModal from "../InfoModal.svelte";
  import { TriangleAlert } from "lucide-svelte";
  import OutputValueDisplay from "./output-value-display.svelte";
  import { pipoio } from "../../pipoio";

  const OSC_ADDRESS_MAX_LENGTH = 32;

  $: config = $currentConfig;
  $: type = $pipoType;
  $: selectedChannel = $uiState[type]?.selectedChannel;
  $: channelType = $uiState[type]?.channelType;

  $: oscConfig =
    config && selectedChannel
      ? (config.engine["engine-osc"][
          selectedChannel as keyof (typeof config.engine)["engine-osc"]
        ] as OscConfig)
      : null;

  // Create a reactive variable for mode_raw so the UI updates when it changes
  $: modeRaw = oscConfig?.mode_raw ?? false;

  $: isChannelEnabled = oscConfig?.enabled ?? false;

  // Single source of truth: display prefix for both reactive display and handler
  $: oscDisplayPrefix =
    "/" +
    (config?.general.PipoName || "") +
    (config?.general.PipoName ? "/" : "");

  // Reactive display values for OSC address
  let oscAddressDisplay = "";
  let quatOscAddressDisplay = "";

  $: if (oscConfig) {
    oscAddressDisplay = oscDisplayPrefix + (oscConfig.osc_addr || "");
  }

  $: if (config?.engine?.["engine-special"]?.quat) {
    quatOscAddressDisplay =
      oscDisplayPrefix + (config.engine["engine-special"].quat.osc_addr || "");
  }

  function toggleEnabled() {
    if (!oscConfig) return;
    oscConfig.enabled = !oscConfig.enabled;
    currentConfig.set(config);
  }

  function onOscAddressInput(event: Event) {
    const input = event.target as HTMLInputElement;
    if (!config) return;

    const prefix = oscDisplayPrefix;
    let raw = input.value;

    // Strip the known display prefix to get the user-editable address part.
    // Three cases:
    //  1. raw starts with full prefix → strip it exactly, get the address
    //  2. raw is a prefix of prefix (user backspaced into the display-only part) → empty address
    //  3. otherwise → treat whole raw value as address (e.g., pasted text)
    let cleaned: string;
    if (raw.startsWith(prefix)) {
      cleaned = raw.substring(prefix.length);
    } else if (prefix.startsWith(raw)) {
      cleaned = "";
    } else {
      cleaned = raw;
    }

    // Remove spaces and special characters
    cleaned = cleaned.replace(/[^a-zA-Z0-9/_\-\.]/g, "");

    // Apply max length limit
    cleaned = cleaned.slice(0, OSC_ADDRESS_MAX_LENGTH);

    // Rebuild display using the same prefix (single source of truth)
    input.value = prefix + cleaned;

    // Update the correct config property (store only the address, no prefix)
    if (channelType === "quaternion") {
      const quat = config.engine?.["engine-special"]?.quat;
      if (quat) {
        quat.osc_addr = cleaned;
      }
    } else if (oscConfig) {
      oscConfig.osc_addr = cleaned;
    }
    currentConfig.set(config);
  }

  function handleOscAddressKeydown(event: KeyboardEvent) {
    // Prevent space and other invalid characters from being entered
    if (event.key === " " || /[^a-zA-Z0-9/_\-\.]/.test(event.key)) {
      // Allow navigation and control keys
      if (
        !event.ctrlKey &&
        !event.metaKey &&
        event.key !== "Backspace" &&
        event.key !== "Delete" &&
        event.key !== "ArrowLeft" &&
        event.key !== "ArrowRight" &&
        event.key !== "Tab"
      ) {
        event.preventDefault();
      }
    }
  }

  // Listen for output values from the device
  let outputValue: number | undefined = undefined;

  pipoio.on("sensor", ({ axis, outputValue: outVal }) => {
    if (axis !== selectedChannel) return;
    outputValue = outVal;
  });
</script>

{#if config && selectedChannel && oscConfig}
  <div class="output-settings">
    <div class="header-row">
      <h4>OSC Output</h4>
      {#if !isChannelEnabled}
        <span class="warning-text">
          <TriangleAlert color="var(--red)" size={14} />
          Channel is disabled
        </span>
      {/if}
    </div>
    <!-- OSC Address -->
    <div class="row">
      <span class="output-label">OSC Address</span>
      <InfoModal>
        <p>
          Defines the OSC address for this channel. It is always prefixed by the
          Pipo Name (in settings pannel). No spaces or special characters are
          allowed.
        </p>
      </InfoModal>
      <div class="input-container">
        <!-- <span> /{config.general.PipoName}/</span> -->
        {#if channelType === "quaternion"}
          <input
            type="text"
            value={quatOscAddressDisplay}
            class="text-input"
            placeholder="address"
            on:input={onOscAddressInput}
            on:keydown={handleOscAddressKeydown}
          />
        {:else}
          <input
            type="text"
            value={oscAddressDisplay}
            class="text-input"
            placeholder="address"
            on:input={onOscAddressInput}
            on:keydown={handleOscAddressKeydown}
          />
        {/if}
      </div>
    </div>

    <!-- Min/Max Values (only when not in raw mode and not quaternion) -->
    {#if !modeRaw && channelType !== "quaternion"}
      <div class="row">
        <span class="output-label">Output Min</span>
        <span></span>
        <div class="input-container">
          <Number
            label=""
            bind:value={oscConfig.osc_min}
            min={0}
            max={oscConfig.osc_max}
          />
        </div>
      </div>

      <div class="row">
        <span class="output-label">Output Max</span>
        <span></span>
        <div class="input-container">
          <Number
            label=""
            bind:value={oscConfig.osc_max}
            min={oscConfig.osc_min}
          />
        </div>
      </div>
    {/if}

    <!-- Output Value Display -->
    <OutputValueDisplay value={outputValue} type="osc" label="OSC Value" />
  </div>
{/if}

<style>
  .output-settings {
    width: 100%;
  }

  .header-row {
    display: flex;
    align-items: center;
    justify-content: flex-start;
    gap: 16px;
    margin-bottom: 8px;
    min-height: 32px;
  }

  .header-row h4 {
    margin: 0;
    line-height: 1;
  }

  .warning-text {
    color: var(--red);
    font-size: 13px;
    font-weight: 600;
    display: flex;
    align-items: center;
    gap: 6px;
    line-height: 1;
    padding: 4px 8px;
    background-color: rgba(255, 0, 0, 0.1);
    border-radius: 4px;
  }

  .row {
    display: grid;
    grid-template-columns: auto 24px 1fr;
    align-items: center;
    gap: 0.5em;
    margin-bottom: 4px;
  }

  .output-label {
    white-space: nowrap;
  }

  /* Input Container */
  .input-container {
    justify-self: end;
  }

  .input-container :global(.input) {
    display: flex;
    flex-direction: row;
    align-items: center;
    gap: 8px;
  }

  .input-container :global(.input label) {
    display: none;
  }

  .input-container :global(.input-wrapper) {
    width: auto;
    min-width: 80px;
  }

  /* Text Input */
  .text-input {
    /* width: 150px; */
    /* height: 32px; */
    padding: 0 12px;
    border: 2px solid var(--main);
    border-radius: 6px;
    background-color: var(--bg-tertiary);
    color: var(--main);
    font-size: 14px;
    font-weight: 500;
  }

  .text-input:focus {
    outline: none;
    border-color: var(--accent);
  }

  .text-input::placeholder {
    color: var(--grey);
    opacity: 0.5;
  }
</style>
