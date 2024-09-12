<script lang="ts">
  import { onDestroy, onMount } from "svelte";
  import { pipoInput } from "../pipoinput";
  let live = false;

  pipoInput
    .on("connect", () => {
      live = true;
    })
    .on("disconnect", () => {
      live = false;
    });
</script>

<nav>
  <span class="status {live ? 'live' : ''}"> </span>
</nav>

<style>
  nav {
    position: fixed;
    top: 1em;
    right: 1em;
    display: flex;
    justify-content: flex-end;
    padding: 0.5em;
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
  }
</style>
