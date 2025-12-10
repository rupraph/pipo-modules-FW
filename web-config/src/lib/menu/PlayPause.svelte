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

  async function setPause() {
    try {
      await pipoio.post("/pause");
      console.log("Toggling pause...");
      // Fetch actual state from backend to ensure sync
      await fetchPauseState();
    } catch (e) {
      console.error("Failed to pause:", e);
      // Revert to actual backend state on error
      await fetchPauseState();
    }
  }

  async function setResume() {
    try {
      await pipoio.post("/resume");
      console.log("Resuming...");
      // Fetch actual state from backend to ensure sync
      await fetchPauseState();
    } catch (e) {
      console.error("Failed to resume:", e);
      // Revert to actual backend state on error
      await fetchPauseState();
    }
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
