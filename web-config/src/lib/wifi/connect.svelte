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
        const [ssid, signal] = line.split(" ");
        const quality = (parseInt(signal) * 100) / 70;
        console.log({ ssid, quality });
        return {
          ssid,
          // TODO: Check this magic numbers from AI,
          // I just have one WIFI around me
          quality: quality < -75 ? "good" : quality < -50 ? "ok" : "bad",
        };
      })
      .filter((e) => e.ssid);
  });
  let connecting = Promise.resolve();
  function onInput(evt) {
    password = evt.target.value;
  }
  function onClick(ssid: string) {
    editing = ssid;
    console.log("Editing", ssid);
  }
  function hideShowPassword() {
    showPassword = !showPassword;
  }
  function onConnect() {
    waiting = true;
    connecting = axios(
      {
        method: "post",
        url: "/wifi-connect",
        params: { ssid: editing, password },
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
      {#each networks as { ssid, quality }}
        <li on:click={() => onClick(ssid)}>
          <span>{ssid}</span>
          <div class="signal {quality}"></div>
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
                  width="35"
                  height="35"
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

            <button on:click={onConnect}>connect</button>
          </div>
        {/if}
      {/each}
    </ul>
  {/await}
</section>

<style scoped>
  ul {
    display: flex;
    flex-direction: column;
    gap: 1em;
  }
  li {
    display: flex;
    gap: 1em;
    width: 100%;
    justify-content: space-between;
    list-style: none;
    align-items: flex-end;
    cursor: pointer;
  }
  li:hover {
    text-decoration: underline;
  }
  .connect {
    display: flex;
    flex-direction: row;
    gap: 1em;
    align-items: center;
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
