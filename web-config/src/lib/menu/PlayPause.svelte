<script lang="ts">
  import MenuButton from "./MenuButton.svelte";
  import Modal from "../modal.svelte";
  import {CirclePause} from "lucide-svelte";

  let open = false;
</script>


<MenuButton on:click={() => open = !open}>
 <CirclePause color="var(--grey)"/>
</MenuButton>

<Modal bind:open={open}>
  Coucou
</Modal>

<style>
</style>
