<script lang="ts">
  import { get } from "svelte/store";
  import Signal from "./signal.svelte";
  import { slide } from "svelte/transition";
  import { Lock, LockOpen, Eye, EyeOff, CircleCheck } from "lucide-svelte";
  import { addToast, type Toast } from "../toast";
  import Modal from "../modal.svelte";
  import { pipoio } from "../../pipoio";
  import { pipoInfo } from "../../services";
  import { setLastScan, wifiState } from "./store";
  import Spinner from "../spinner.svelte";
  import type { Network } from "./types";
  import { fetchNetworks, fetchState } from "../../services/wifi";
  import { onMount } from "svelte";
  import { encodePassword } from "./encoding";

  let editing = "";
  let showPassword = false;
  let password: string | undefined = undefined;
  let waiting = false;
  let wifiMode = "";
  let mounted = false;
  let showConnectedWarning = false;

  const WIFI_PASSWORD_MAX_LENGTH = 63; // WPA/WPA2 standard max length

  // Use reactive declarations for better Svelte reactivity
  $: networks = $wifiState.networks;
  $: apIP = $wifiState.apIP;
  $: staIP = $wifiState.staIP;
  $: if (mounted) onShow();

  onMount(() => {
    mounted = true;
  });

  async function onShow() {
    const now = Date.now();
    if (get(wifiState).lastScan < now - 30000) {
      await scan();
    }
  }
  export async function scan() {
    if (waiting) return;
    await pipoio.pause();
    waiting = true;
    let toast = {
      type: "info" as const,
      message: `Scanning for networks...`,
      timeout: 2000,
    };
    addToast(toast);
    try {
      await pipoio.request({
        method: "post",
        url: "/wifi-start-scan",
      });
    } catch (e) {
      // if we are already scanning, we will get a 503
      console.error(e);
    }
    try {
      // wait for the scan to complete
      await new Promise((resolve) => setTimeout(resolve, 1000));
      await fetchNetworks();
      await pipoio.resume();
      setLastScan(Date.now());
      waiting = false;
    } catch (e) {
      await pipoio.resume();
      waiting = false;
      console.error(e);
    }
  }
  function onInput(evt: Event) {
    const input = evt.target as HTMLInputElement;
    let value = input.value;

    // Apply max length limit (WPA/WPA2 standard)
    if (value.length > WIFI_PASSWORD_MAX_LENGTH) {
      value = value.slice(0, WIFI_PASSWORD_MAX_LENGTH);
      input.value = value;
    }

    password = value;
  }
  function onLockClick(ssid: string, known: boolean) {
    if (!known) return;
    password = "";
    editing = ssid;
  }
  function onSelect(ssid: string, known: boolean) {
    const current = networks.find((n) => n.connected);
    if (waiting || (current && ssid === current.ssid)) return;

    // Check if already connected to a different network
    if (current && ssid !== current.ssid) {
      showConnectedWarning = true;
      return;
    }

    if (known) {
      password = undefined;
      return onConnect(ssid);
    }
    password = "";
    editing = ssid;
  }
  function hideShowPassword() {
    showPassword = !showPassword;
  }
  async function setMode(mode: string) {
    if (waiting) return;
    waiting = true;
    let toast = {
      type: "info" as const,
      message: `Setting mode to ${mode}...`,
      timeout: 5000,
    };
    addToast(toast);
    try {
      await pipoio.request({
        method: "post",
        url: "/wifi-mode",
        params: { mode },
        timeout: 1000,
      });
      await new Promise((resolve) => setTimeout(resolve, 5000));
      await fetchNetworks();
      await fetchState();
    } catch (e) {
      console.error(e);
    }
    waiting = false;
  }
  async function onForget(ssid: string) {
    if (waiting) return;
    waiting = true;
    let toast: Toast = {
      type: "info",
      message: `Forgetting ${ssid}...`,
      timeout: 3000,
    };
    addToast(toast);
    try {
      await pipoio.request({
        method: "post",
        url: "/wifi-forget",
        params: { ssid },
        timeout: 1000,
      });
      console.log("Forgotten wifi", ssid);
      toast = {
        type: "success",
        message: `Network ${ssid} forgotten`,
        timeout: 3000,
      };
      await fetchNetworks();
    } catch (e) {
      console.error(e);
      toast = {
        type: "error",
        message: `Failed to forget ${ssid}`,
        timeout: 3000,
      };
    }
    waiting = false;
    editing = "";
    addToast(toast);
  }
  async function onDisconnect() {
    if (waiting) return;
    waiting = true;
    let toast: Toast = {
      type: "info",
      message: `Disconnecting from WiFi...`,
      timeout: 3000,
    };
    addToast(toast);

    // Pause websocket before disconnect
    await pipoio.pause();

    try {
      await pipoio.request({
        method: "post",
        url: "/wifi-disconnect",
        timeout: 1000,
      });
      // Wait 3s for disconnection
      await new Promise((resolve) => setTimeout(resolve, 3000));
    } catch (e) {
      console.error(e);
    }

    // Reload page to show new state
    location.reload();
  }
  async function onConnect(ssid: string) {
    if (waiting) return;
    waiting = true;
    let toast: Toast = {
      type: "info",
      message: `Connecting to ${ssid}...`,
      timeout: 5000,
    };
    addToast(toast);

    // Pause websocket before network change since we'll reload anyway
    await pipoio.pause();

    try {
      // Encode password using MAC address for basic obfuscation
      const info = get(pipoInfo);
      const encodedPassword =
        info && password ? encodePassword(password, info.mac) : password;

      await pipoio.request({
        method: "post",
        url: "/wifi-connect",
        params: {
          ssid,
          password: encodedPassword || "",
          encoded: encodedPassword ? "true" : "false",
        },
        timeout: 1000,
      });
      // Wait 5s for network connection to establish
      await new Promise((resolve) => setTimeout(resolve, 5000));
    } catch (e) {
      console.error(e);
    }

    // Network changed - reload page to get new IP
    location.reload();
  }
</script>

<section class="connection" class:waiting>
  <h3 style="text-align: center;">WiFi Networks</h3>
  <!-- <div class="ips">
    <span><strong>APIP:</strong> {apIP}</span>
  </div> -->
  <div
    style="display: flex; gap: 8px; width: 100%; align-items: center; justify-content: center;"
  >
    <button class="primary" class:disabled={waiting} on:click={() => scan()}
      >Scan</button
    >
    {#if networks.find((n) => n.connected)}
      <button
        class="secondary"
        class:disabled={waiting}
        on:click={onDisconnect}
      >
        Disconnect
      </button>
    {/if}
  </div>
  {#if waiting && networks.length === 0}
    <Spinner />
    <p>Scanning for networks...</p>
  {:else if waiting}
    <Spinner />
    <p>Waiting for networks...</p>
  {:else}
    {#if networks.length === 0}
      <p>No networks found</p>
    {:else}
      <ul>
        {#each networks as { ssid, quality, known, connected }}
          <li on:click={() => onSelect(ssid, known)}>
            <div class="ssid-container">
              <span class="ssid">{ssid}</span>
              {#if connected && staIP}
                <span class="sta-ip">Pipo IP: {staIP}</span>
              {/if}
            </div>
            {#if connected}
              <CircleCheck size={24} class="checkmark" />
            {:else}
              <span></span>
            {/if}
            <button class="lock-icon" on:click={() => onLockClick(ssid, known)}>
              {#if known}
                <LockOpen size={20} />
              {:else}
                <Lock size={20} />
              {/if}
            </button>
            {#if connected}
              <Signal signal={quality} bars={5} />
            {:else}
              <span></span>
            {/if}
            {#if known}
              <button class="forget" on:click={() => onForget(ssid)}
                >forget</button
              >
            {:else}
              <span></span>
            {/if}
          </li>
          {#if editing === ssid}
            <div
              class="connect"
              transition:slide={{
                delay: 0,
                duration: 300,
                axis: "y",
              }}
            >
              <label> password </label>
              <input
                class="input-wrapper"
                on:input={onInput}
                type={showPassword ? "text" : "password"}
                id="network-password"
                maxlength={WIFI_PASSWORD_MAX_LENGTH}
              />
              <button class="showhide" on:click={hideShowPassword}>
                {#if showPassword}
                  <Eye size={24} />
                {:else}
                  <EyeOff size={24} />
                {/if}
              </button>

              <button class="connect" on:click={() => onConnect(editing)}
                >connect</button
              >
            </div>
          {/if}
        {/each}
      </ul>
    {/if}

    <!-- {:else if 0}
    <div class="buttons">
      <button
        class:enabled={wifiMode == "AP"}
        class:disabled={waiting}
        on:click={() => setMode("AP")}>AP mode</button
      >
      <button
        class:enabled={wifiMode == "STA"}
        class:disabled={waiting}
        on:click={() => setMode("STA")}>STA mode</button
      >
      <button
        class:enabled={wifiMode == "APSTA"}
        class:disabled={waiting}
        on:click={() => setMode("APSTA")}>AP_STA mode</button
      >
    </div> -->
  {/if}
</section>

<Modal bind:open={showConnectedWarning}>
  <h3 style="text-align: center;">Already Connected</h3>
  <p style="text-align: center; margin: 1em;">
    You are already connected to a network. Please disconnect first before
    connecting to another network.
  </p>
</Modal>

<style scoped>
  .connection {
    position: relative;
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 1em;
    font-size: 1em;
    padding: 0 1em 0.5em 1em;
    max-height: 80vh;
    overflow-y: auto;

    /* Standard scrollbar styling */
    scrollbar-width: thin;
    scrollbar-color: var(--main) transparent;
  }

  .connection.waiting,
  .connection.waiting * {
    cursor: wait;
    color: var(--bg-lighter);
  }
  .connection > h3 {
    margin: 0.5em 0;
  }
  ul {
    width: 100%;
    padding: 0;
    margin: 0;
    grid-template-columns: minmax(0, 1fr) 1em 1em 2em 4em;
    grid-template-rows: repeat(auto-fill, auto);
    justify-items: start;
    align-items: center;
    display: grid;
    gap: 2em;
    margin-bottom: 1em;
  }
  li {
    display: contents;
    width: 100%;
    list-style: none;
    cursor: pointer;
  }
  .connect {
    grid-column-start: 1;
    grid-column-end: 6;
    display: flex;
    flex-direction: row;
    gap: 1em;
    align-items: center;
  }
  .ssid-container {
    display: flex;
    flex-direction: column;
    gap: 0.2em;
    max-width: 100%;
  }
  .ssid {
    overflow-wrap: break-word;
    word-break: break-word;
    hyphens: auto;
    max-width: 100%;
  }
  .sta-ip {
    font-size: 0.8em;
    color: var(--text-color);
    opacity: 0.7;
  }
  button.lock-icon {
    width: fit-content;
    background-color: transparent;
    border: none;
    cursor: pointer;
    padding: 0;
    color: var(--text-color);
  }
  button.lock-icon:hover {
    color: var(--main);
  }
  :global(.checkmark) {
    color: #8fbe00;
  }
  button.showhide {
    width: fit-content;
    background-color: transparent;
    border: none;
    cursor: pointer;
    padding: 0;
    color: var(--text-color);
  }
  button.showhide:hover {
    color: var(--main);
  }
  button.enabled {
    background-color: var(--main);
    color: var(--bg-lighter);
  }
  button.forget {
    background-color: #dc3545;
    color: white;
    padding: 6px;
    height: fit-content;
  }

  /* button.connect {
    height: fit-content;
  } */
  button.forget:hover {
    background-color: #c82333;
  }
  .buttons {
    display: flex;
    flex-direction: row;
    gap: 1em;
  }
  .ips {
    display: flex;
    flex-direction: row;
    flex-wrap: wrap;
    gap: 1em;
  }

  .input-wrapper {
    border-color: var(--main);
  }
</style>
