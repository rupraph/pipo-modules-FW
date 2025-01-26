<script lang="ts">
  import Signal from "./signal.svelte";
  import axios from "axios";
  import { slide } from "svelte/transition";
  import { addToast, type Toast } from "../toast";
  import { pipoio } from "../../pipoio";
  import { wifiState } from "./store";
  type Network = {
    ssid: string;
    quality: number;
    known: boolean;
    connected: boolean;
  };
  let editing = "";
  let showPassword = false;
  let password: string | undefined = undefined;
  let waiting = false;
  let connecting = Promise.resolve();
  let wifiMode = "";
  let networks: Network[] = [];
  $: fetchNetworks();
  $: fetchMode();

  async function fetchMode() {
    let retry = 0;
    const maxRetry = 5;
    while (retry++ < maxRetry) {
      try {
        wifiMode = (await axios.get("/wifi-state")).data.split(" ")[0];
        break;
      } catch (e) {
        console.error(e);
      }
    }
  }
  async function fetchNetworks() {
    // debugger;
    // await axios.get("/wifi-scan", { timeout: 5000 });
    let { data } = await axios.get("/wifi-networks");
    // let data = "";
    // data += `Freebox-3443AA_EXT -87 0 0`;
    console.log("connect", data);
    networks = (data as string)
      .trim()
      .split("\n")
      .map((line) => {
        const [_, ssid, signal, connected, known] = line.match(
          /"(.*)" (-?\d+) (\d+) (\d+)/
        );

        let quality = parseInt(signal);
        if (isNaN(quality)) {
          quality = -100;
        }
        // TODO: Check this magic numbers from AI,
        quality = Math.max(-100, Math.min(quality, -50));
        quality = (-quality - 50) / 50;
        return {
          ssid,
          known: known === "1",
          connected: connected === "1",
          quality,
        };
      })
      .filter((e) => e.ssid)
      .sort((a, b) => {
        if (a.connected) return -1;
        if (b.connected) return 1;
        if (a.known && !b.known) return -1;
        if (!a.known && b.known) return 1;
        return a.quality - b.quality;
      });
    const connected = networks.find((n) => n.connected);
    if (connected) {
      wifiState.set({ signal: connected.quality, ssid: connected.ssid });
    } else {
      wifiState.set({ signal: 0, ssid: "" });
    }
  }
  async function scan() {
    if (waiting) return;
    waiting = true;
    let toast = {
      type: "info" as const,
      message: `Scanning for networks...`,
      timeout: 5000,
    };
    addToast(toast);
    try {
      await axios(
        {
          method: "post",
          url: "/wifi-scan",
        },
        { timeout: 1000 }
      );
      await fetchNetworks();
    } catch (e) {
      console.error(e);
    }
  }
  function onInput(evt) {
    password = evt.target.value;
  }
  function onLockClick(ssid: string, known: boolean) {
    if (!known) return;
    // axios.post("/wifi-forget", { ssid });
    password = "";
    editing = ssid;
    fetchNetworks();
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
      await axios(
        {
          method: "post",
          url: "/wifi-mode",
          params: { mode },
        },
        { timeout: 1000 }
      );
      await new Promise((resolve) => setTimeout(resolve, 5000));
      await fetchNetworks();
      await fetchMode();
    } catch (e) {
      console.error(e);
    }
    waiting = false;
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
      await axios(
        {
          method: "post",
          url: "/wifi-connect",
          params: { ssid, password },
        },
        { timeout: 1000 }
      );
      await new Promise((resolve) => setTimeout(resolve, 5000));
    } catch (e) {
      console.error(e);
    }
    while (retry++ < maxRetry) {
      try {
        const [mode, status, ...info] = (
          await axios.get("/wifi-state")
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
            console.log("Fallback to", newssid);
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
      await fetchMode();
      await fetchNetworks();
    } catch (e) {
      console.error(e);
    }
    pipoio.resume();
    waiting = false;
    editing = "";
    console.log("END TOAST", toast);
    addToast(toast);
    // await fetchNetworks();
  }
</script>

<section class="connection" class:waiting>
  {#await networks}
    <p>Searching for networks...</p>
  {:then networks}
    <h3>Available networks</h3>
    <ul>
      {#each networks as { ssid, quality, known, connected }}
        <li on:click={() => onSelect(ssid, known)}>
          <span>{ssid}</span>
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
          <Signal signal={5 - Math.floor(quality * 5)} />
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

            <button on:click={onConnect(editing)}>connect</button>
          </div>
        {/if}
      {/each}
    </ul>
    {#if 0}
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
      </div>
    {/if}
  {/await}
</section>

<style scoped>
  .connection {
    position: relative;
    display: grid;
    gap: 1em;
    margin-bottom: 2em;
  }
  .connection.waiting,
  .connection.waiting * {
    cursor: wait;
    color: var(--bg-lighter);
  }
  .connection > h3 {
    margin: 1em 0;
  }
  ul {
    margin: 0;
    grid-template-columns: auto 3em 3em 3em;
    justify-items: start;
    align-items: end;
    display: grid;
    gap: 1em;
  }
  li {
    display: contents;
    width: 100%;
    list-style: none;
    cursor: pointer;
  }
  section:not(.waiting) li:hover > span {
    text-decoration: underline;
  }
  .connect {
    margin-bottom: 2em;
    grid-column-start: 1;
    grid-column-end: 5;
    display: flex;
    flex-direction: row;
    gap: 1em;
    align-items: center;
  }
  svg.lock {
    fill: var(--bg-lighter);
  }
  button.showhide {
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
  .buttons {
    display: flex;
    flex-direction: row;
    gap: 1em;
  }
</style>
