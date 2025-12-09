<script lang="ts">
  import { onMount } from "svelte";
  import MenuButton from "./MenuButton.svelte";
  import { CirclePause, CirclePlay } from "lucide-svelte";
  import { pipoio } from "../../pipoio";

  let isPaused = false;

  async function fetchPauseState() {
    try {
      const response = await pipoio.get("/is-paused");
      isPaused =
        response.data === "true" ||
        response.data === true ||
        response.data === 1;
    } catch (e) {
      console.error("Failed to fetch pause state:", e);
    }
  }

  function setPause() {
    pipoio.post("/pause").then(() => {
      console.log("Toggling pause...");
    });
    isPaused = true;
  }

  function setResume() {
    pipoio.post("/resume").then(() => {
      console.log("Resuming...");
    });
    isPaused = false;
  }

  function togglePause() {
    if (isPaused) {
      setResume();
    } else {
      setPause();
    }
  }

  onMount(() => {
    fetchPauseState();
  });
</script>

<MenuButton on:click={togglePause}>
  {#if isPaused}
    <CirclePlay color="var(--grey)" size={50} />
  {:else}
    <CirclePause color="var(--grey)" size={50} />
  {/if}
</MenuButton>

<style>
</style>
