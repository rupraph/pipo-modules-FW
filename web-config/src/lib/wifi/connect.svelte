<script lang="ts">
  import { get } from "svelte/store";
  import Signal from "./signal.svelte";
  import { slide } from "svelte/transition";
  import { addToast, type Toast } from "../toast";
  import { pipoio } from "../../pipoio";
  import { setLastScan, wifiState } from "./store";
  import Spinner from "../spinner.svelte";
  import type { Network } from "./types";
  import { fetchNetworks, fetchState } from "../../services/wifi";

  let editing = "";
  let showPassword = false;
  let password: string | undefined = undefined;
  let waiting = false;
  let wifiMode = "";
  let networks: Network[];
  let apIP = "";
  let staIP = "";
  wifiState.subscribe((v) => {
    networks = v.networks;
    apIP = v.apIP;
    staIP = v.staIP;
  });
  $: onShow();
  async function onShow() {
    const now = Date.now();
    if (get(wifiState).lastScan < now - 30000) {
      await scan();
    }
  }
  export async function scan() {
    if (waiting) return;
    pipoio.pause();
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
      pipoio.resume();
      setLastScan(Date.now());
      waiting = false;
    } catch (e) {
      pipoio.resume();
      console.error(e);
    }
  }
  function onInput(evt: Event) {
    password = (evt.target as HTMLInputElement).value;
  }
  function onLockClick(ssid: string, known: boolean) {
    if (!known) return;
    password = "";
    editing = ssid;
  }
  function onSelect(ssid: string, known: boolean) {
    const current = networks.find((n) => n.connected);
    if (waiting || (current && ssid === current.ssid)) return;
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
  async function onConnect(ssid: string) {
    if (waiting) return;
    waiting = true;
    let retry = 0;
    const maxRetry = 5;
    let toast: Toast = {
      type: "info",
      message: `Connecting to ${ssid}...`,
      timeout: 5000,
    };
    addToast(toast);
    await pipoio.pause();
    try {
      await pipoio.request({
        method: "post",
        url: "/wifi-connect",
        params: { ssid, password },
        timeout: 1000,
      });
      await new Promise((resolve) => setTimeout(resolve, 5000));
    } catch (e) {
      console.error(e);
    }
    return location.reload();
    while (retry++ < maxRetry) {
      try {
        const [mode, status, ...info] = (
          await pipoio.get("/wifi-state")
        ).data.split(" ");
        wifiMode = mode;
        if (status === "CONNECTING") continue;
        if (status === "UNKNOWN") {
          toast = {
            type: "error",
            message: `Connection to ${ssid} return unexpected error, please restart PIPO`,
            timeout: 5000,
          };
          break;
        }
        if (status === "CONNECTED") {
          const [ip, newssid] = info;
          if (newssid === ssid) {
            toast = {
              type: "success",
              message: `Connected to ${ssid} with IP ${ip}`,
              timeout: 5000,
            };
          } else if (newssid) {
            toast = {
              type: "warning",
              message: `Could not connect to ${ssid}, fallback on ${newssid}`,
              timeout: 5000,
            };
          } else {
            toast = {
              type: "error",
              message: `Could not connect to ${ssid}, fallback on AP mode`,
              timeout: 5000,
            };
          }
          break;
        }
      } catch (e) {
        console.error(e);
      }
      await new Promise((resolve) => setTimeout(resolve, 2000));
    }
    try {
      await fetchNetworks();
    } catch (e) {
      console.error(e);
    }
    pipoio.resume();
    waiting = false;
    editing = "";
    addToast(toast);
  }
</script>

<section class="connection" class:waiting>
  <h3>Networks</h3>
  <div class="ips">
    <span><strong>APIP:</strong> {apIP}</span>
    <span><strong>STAIP:</strong> {staIP}</span>
  </div>
  <button class="primary" class:disabled={waiting} on:click={() => scan()}
    >Scan</button
  >
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
            <span class="ssid">{ssid}</span>
            <svg
              class="lock"
              height="30"
              width="30"
              xmlns="http://www.w3.org/2000/svg"
              viewBox="0 0 100 100"
              stroke="black"
              stroke-width="6"
              on:click={() => onLockClick(ssid, known)}
            >
              <g>
                <path
                  stroke-linecap="round"
                  fill="none"
                  d="M25 50 L75 50 75 95 25 95 25 50"
                />
                <circle cx="50" cy="75" r="5" fill="none" />
                <path
                  stroke-linecap="round"
                  fill="none"
                  d={known
                    ? "M40 50 L40 30 C 40 0 3 0 3 30 L 3 35"
                    : "M30 50 L30 30 C 30 0 70 0 70 30 L 70 50"}
                />
              </g>
            </svg>
            {#if connected}
              <svg
                class="checkmark"
                height="30"
                width="30"
                xmlns="http://www.w3.org/2000/svg"
                viewBox="0 0 100 100"
                xml:space="preserve"
                stroke="#8fbe00"
                stroke-width="6"
              >
                <g>
                  <path
                    stroke-linecap="round"
                    fill="none"
                    d="M30 75 L50 90 90 30"
                  />
                </g>
              </svg>
            {:else}
              <span></span>
            {/if}
            <Signal signal={quality} bars={5} />
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
              />
              <button class="showhide stroke" on:click={hideShowPassword}>
                <icon>
                  <svg
                    xmlns="http://www.w3.org/2000/svg"
                    width="30"
                    height="30"
                    viewBox="0 0 24 24"
                    ><path
                      d="M15 12c0 1.654-1.346 3-3 3s-3-1.346-3-3 1.346-3 3-3 3 1.346 3 3zm9-.449s-4.252 8.449-11.985 8.449c-7.18 0-12.015-8.449-12.015-8.449s4.446-7.551 12.015-7.551c7.694 0 11.985 7.551 11.985 7.551zm-7 .449c0-2.757-2.243-5-5-5s-5 2.243-5 5 2.243 5 5 5 5-2.243 5-5z"
                    />
                    {#if !showPassword}
                      <path stroke-width="3" d="M0 0 L24 24" stroke="black"
                      ></path>
                    {/if}
                  </svg>
                </icon>
              </button>

              <button on:click={() => onConnect(editing)}>connect</button>
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

<style scoped>
  .connection {
    position: relative;
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 1em;
    font-size: 1em;
    padding: 0 0.5em 0.5em 0.5em;
    max-height: calc(100vh - 10em);
    overflow-y: auto;
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
    grid-template-rows: repeat(auto-fill, 2em);
    justify-items: start;
    align-items: end;
    display: grid;
    gap: 1em;
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
    grid-column-end: 5;
    display: flex;
    flex-direction: row;
    gap: 1em;
    align-items: center;
  }
  .ssid {
    text-overflow: ellipsis;
    overflow: hidden;
    white-space: nowrap;
    max-width: 100%;
  }
  svg.lock {
    fill: var(--bg-lighter);
  }
  button.showhide {
    width: fit-content;
    background-color: transparent;
    background-repeat: no-repeat;
    border: none;
    cursor: pointer;
    overflow: hidden;
    outline: none;
  }
  button.showhide svg {
    fill: var(--text-color);
  }
  button.showhide:hover svg {
    fill: var(--main);
  }
  button.enabled {
    background-color: var(--main);
    color: var(--bg-lighter);
  }
  button.forget {
    background-color: #dc3545;
    color: white;
    padding: 10px;
  }
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
</style>
