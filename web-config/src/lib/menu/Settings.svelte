<script lang="ts">
  import MenuButton from "./MenuButton.svelte";
  import Modal from "../modal.svelte";
  import {SettingsIcon} from "lucide-svelte";

  let open = false;
</script>


<MenuButton on:click={() => open = !open}>
 <SettingsIcon color="var(--grey)"/>
</MenuButton>

<Modal bind:open={open}>
  Coucou
</Modal>

<style>
</style>
