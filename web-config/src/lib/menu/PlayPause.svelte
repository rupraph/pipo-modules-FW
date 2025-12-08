<script lang="ts">
  import MenuButton from "./MenuButton.svelte";
  import { CirclePause, CirclePlay } from "lucide-svelte";
  import { pipoio } from "../../pipoio";

  let isPaused = false;

  function togglePause() {
    pipoio.post("/pause").then(() => {
      console.log("Toggling pause...");
    });
    isPaused = !isPaused;
  }
</script>

<MenuButton on:click={togglePause}>
  {#if isPaused}
    <CirclePause color="var(--grey)" size={50} />
  {:else}
    <CirclePlay color="var(--grey)" size={50} />
  {/if}
</MenuButton>

<style>
</style>
