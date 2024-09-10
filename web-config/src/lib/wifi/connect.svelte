<script lang="ts">
  import Input from "../form/Input.svelte";
  import axios from "axios";
  import { slide } from "svelte/transition";
  let editing = "";
  let showPassword = false;
  let password = "";
  let waiting = false;
  const networks = axios.get("/wifi-networks").then(({ data }) => {
    return (data as string)
      .trim()
      .split("\n")
      .map((line) => {
        const [ssid, signal, known, connected] = line.split(" ");
        const quality = parseInt(signal);
        console.log({ ssid, quality });
        return {
          ssid,
          known: known === "1",
          connected: connected === "1",
          // TODO: Check this magic numbers from AI,
          // I just have one WIFI around me
          quality: quality > -65 ? "good" : quality > -55 ? "ok" : "bad",
        };
      })
      .filter((e) => e.ssid);
  });
  let connecting = Promise.resolve();
  function onInput(evt) {
    password = evt.target.value;
  }
  function onSelect(ssid: string, known: boolean) {
    if (known) {
      return onConnect(ssid);
    }
    editing = ssid;
    console.log("Editing", ssid);
  }
  function hideShowPassword() {
    showPassword = !showPassword;
  }
  function onConnect(ssid: string) {
    waiting = true;
    connecting = axios(
      {
        method: "post",
        url: "/wifi-connect",
        params: { ssid, password },
      },
      { timeout: 1000 }
    )
      .catch((e) => {
        return Promise.resolve();
      })
      .then(() => new Promise((resolve) => setTimeout(resolve, 2000)))
      .then(() => axios.get("/wifi-status"))
      .then(({ data }) => {
        const [status, info] = data.split(" ");
        if (status === "connected") {
          editing = "";
        }
      })
      .finally(() => {
        waiting = false;
        editing = "";
      });
  }
</script>

<section class="connection">
  {#await networks}
    <p>Searching for networks...</p>
  {:then networks}
    <h3>Available networks</h3>
    <ul>
      {#each networks as { ssid, quality, known, connected }}
        <li on:click={() => onSelect(ssid, known)}>
          <span>{ssid}</span>
          <div class="signal {quality}"></div>
          <svg
            class="lock"
            height="30"
            width="30"
            xmlns="http://www.w3.org/2000/svg"
            viewBox="0 0 100 100"
            stroke="white"
            stroke-width="6"
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
              stroke="white"
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
  {/await}
</section>

<style scoped>
  ul {
    grid-template-columns: auto 3em 3em 1em;
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
  li:hover > span {
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

  /* HTML: <div class="signal"></div> */
  .signal {
    --n: 5; /* the number of bars */
    --g: 30%; /* control the gap */

    width: 3em;
    aspect-ratio: 1.5;
    mask:
      linear-gradient(-90deg, #0000 var(--g), #000 0) 0 / calc(100% / var(--n))
        intersect,
      linear-gradient(to top left, #000 50%, #0000 0),
      linear-gradient(to top left, #000 calc(50% + 50% / var(--n)), #0000 0)
        intersect,
      repeating-conic-gradient(#000 0 25%, #0000 0 50%) 0 100% /
        calc(200% / var(--n)) calc(200% / var(--n));
  }
  .signal.good {
    background: #8fbe00;
  }
  .signal.ok {
    background: #f7b500;
  }
  .signal.bad {
    background: #f70000;
  }
</style>
