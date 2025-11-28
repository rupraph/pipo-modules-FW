<script lang="ts">
  import { onMount, onDestroy } from "svelte";
  import Collapse from "../collapse.svelte";
  import soundIcon from "../../assets/sound.svg";
  import muteIcon from "../../assets/mute.svg";
  import { sound } from "./sound";
  import Checkbox from "../form/Checkbox.svelte";
  import { pipoio } from "../../pipoio";
  let error: string = "";
  let isMute = true;
  let autoScroll = true;
  const notes = new Array(127).fill(0).map((_, i) => i);
  const names = [
    "B",
    "C",
    "CS",
    "D",
    "DS",
    "E",
    "F",
    "FS",
    "G",
    "GS",
    "A",
    "AS",
  ];
  const playing = new Array(notes.length).fill(false);
  let timeout: ReturnType<typeof setTimeout>;
  function scrollTo(note: number) {
    if (!autoScroll) return;
    const element = document.querySelector(
      `.keyboard li:nth-child(${note + 1})`
    );
    if (!element) return;
    if (timeout) clearTimeout(timeout);
    timeout = setTimeout(() => {
      element.scrollIntoView({
        behavior: "smooth",
        block: "center",
        inline: "center",
      });
    }, 50);
  }

  onMount(async () => {
    pipoio.on("noteOn", ({ note }) => {
      playing[note] = true;
      sound.noteOn(note);
      scrollTo(note);
    });
    pipoio.on("noteOff", ({ note }) => {
      playing[note] = false;
      sound.noteOff(note);
    });
  });

  onDestroy(() => {
    sound.stop();
  });
</script>

<Collapse title="Keyboard" class="keyboard" open collapseId="piano">
  <div class="layout">
    <div class="piano scroll">
      <ul class="keyboard">
        {#each notes as i}
          {#if names[i % 12].endsWith("S")}
            <li class="black {names[i % 12]} {playing[i] ? 'active' : ''}"></li>
          {:else}
            <li class="white {names[i % 12]} {playing[i] ? 'active' : ''}"></li>
          {/if}
        {/each}
      </ul>
      {#if error}
        <div class="error">
          <p>{error}</p>
        </div>
      {/if}
    </div>
    <button
      class="sound-button"
      on:click={() => {
        isMute ? sound.play() : sound.pause();
        isMute = !isMute;
      }}
    >
      <img src={isMute ? soundIcon : muteIcon} alt="sound" />
    </button>

    <Checkbox class="autoscroll" bind:value={autoScroll} label="Auto Scroll" />
  </div>
</Collapse>

<style>
  :global(section.keyboard) {
    display: flex;
    justify-content: center;
  }
  :global(.autoscroll) {
    display: flex;
    gap: 1em;
  }

  .layout {
    display: grid;
    grid-template-columns: auto 1em;
    grid-template-rows: 1em auto;
    max-width: 100%;
    gap: 1em;
  }
  .piano {
    grid-area: 1 / 1 / 3 / 3;
    display: grid;
    grid-template-columns: 100%;
    grid-template-rows: 100%;
    overflow-x: scroll;
  }
  .sound-button {
    z-index: 3;
    grid-area: 1 / 1 / 2 / 2;
    padding: 1.5em;
    width: 1em;
    height: 1em;
    border-radius: 50%;
    border: 1px solid var(--bg-color);
  }
  .sound-button:hover {
    border-color: var(--main);
  }
  .error {
    grid-row: 1;
    grid-column: 1;
    background-color: rgba(127, 127, 127, 0.7);
    color: var(--red);
    font-size: 2em;
    z-index: 10;
    display: flex;
    align-items: center;
    justify-content: space-around;
  }
  ul.keyboard {
    grid-row: 1;
    grid-column: 1;
    display: flex;
    margin: 0;
    padding: 0;
    width: fit-content;
    margin: 1em;
  }

  li {
    box-sizing: border-box;
    margin: 0;
    padding: 0;
    list-style: none;
    float: left;
  }

  ul .white {
    height: 16em;
    width: 4em;
    z-index: 1;
    border-left: 1px solid #bbb;
    border-bottom: 1px solid #bbb;
    border-radius: 0 0 5px 5px;
    box-shadow:
      -1px 0 0 rgba(255, 255, 255, 0.8) inset,
      0 0 5px #ccc inset,
      0 0 3px rgba(0, 0, 0, 0.2);
    background: linear-gradient(to bottom, #eee 0%, #fff 100%);
  }

  ul .white.active {
    border-top: 1px solid #777;
    border-left: 1px solid #999;
    border-bottom: 1px solid #999;
    box-shadow:
      2px 0 3px rgba(0, 0, 0, 0.1) inset,
      -5px 5px 20px rgba(0, 0, 0, 0.5) inset,
      0 0 3px rgba(0, 0, 0, 0.2);
    background: linear-gradient(to bottom, #fff 0%, #e9e9e9 100%);
  }

  .black {
    height: 8em;
    width: 2em;
    margin: 0 0 0 -1em;
    z-index: 2;
    border: 1px solid #000;
    border-radius: 0 0 3px 3px;
    box-shadow:
      -1px -1px 2px rgba(255, 255, 255, 0.2) inset,
      0 -5px 2px 3px rgba(0, 0, 0, 0.6) inset,
      0 2px 4px rgba(0, 0, 0, 0.5);
    background: linear-gradient(45deg, #222 0%, #555 100%);
  }

  .black.active {
    box-shadow:
      -1px -1px 2px rgba(255, 255, 255, 0.2) inset,
      0 -2px 2px 3px rgba(0, 0, 0, 0.6) inset,
      0 1px 2px rgba(0, 0, 0, 0.7);
    background: linear-gradient(to right, #444 0%, #222 100%);
  }

  .A,
  .AS,
  .G,
  .GS,
  .E,
  .D,
  .DS,
  .B,
  .C,
  .CS {
    margin: 0 0 0 -1em;
  }

  ul li:first-child {
    border-radius: 5px 0 5px 5px;
  }

  ul li:last-child {
    border-radius: 0 5px 5px 5px;
  }
</style>
