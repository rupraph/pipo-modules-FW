<script lang="ts">
  import { pipoInput } from "../pipoinput";
  import { isLive } from "../services";
  let live = false;
  isLive.subscribe((value) => {
    live = value;
  });
  let fps = 0;
  const max = 10;
  let last = 0;
  const dts: number[] = new Array(max).fill(0);
  pipoInput
    .on("fps", ({ frames, dt }) => {
      dts[last++ % max] = frames / dt;
      fps = Math.round((dts.reduce((a, b) => a + b, 0) / max) * 1000);
    })
    .on("disconnect", () => {
      fps = 0;
    });
</script>

<nav>
  <span class="status {live ? 'live' : ''}"> </span>
  <span>FPS: {fps}</span>
</nav>

<style>
  nav {
    width: 100%;
    display: flex;
    padding: 0.5em;
    gap: 1em;
    align-items: center;
    box-sizing: border-box;
  }
  .status {
    width: 1em;
    height: 1em;
    border-radius: 50%;
    background-color: var(--red);
    transition: all 1.5s;
    filter: drop-shadow(0 0 0.5em var(--red));
  }
  .status.live {
    background-color: var(--green);
    filter: drop-shadow(0 0 0.5em var(--green));
    margin-left: auto;
  }
</style>
