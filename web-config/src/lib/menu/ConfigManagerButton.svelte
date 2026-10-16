<script lang="ts">
  import MenuButton from "./MenuButton.svelte";
  import ConfigManager from "../config-manager/ConfigManager.svelte";
  import { Files } from "lucide-svelte";

  let open = false;
</script>

<MenuButton on:click={() => (open = !open)}>
  <Files color="var(--grey)" size={20} />
</MenuButton>

<ConfigManager bind:open />
