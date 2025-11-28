<script lang="ts">
  import Config from "./config.svelte";
  import Tabs from "./tabs.svelte";
  import { onMount } from "svelte";
  import {
    configService,
    configNames,
    activeConfigName,
    currentConfig,
    configsLoading,
    configsError,
  } from "../../services/config";

  onMount(() => {
    configService.initialize();
  });

  async function onClick(name: string) {
    try {
      await configService.setActiveConfig(name);
    } catch (err) {
      console.error("Failed to switch config:", err);
    }
  }

  async function onCreate() {
    const names = $configNames;
    const index = names.length + 1;
    try {
      await configService.createConfig(`Config-${index}`);
    } catch (err) {
      console.error("Failed to create config:", err);
    }
  }

  async function onRename(oldname: string, newname: string) {
    try {
      await configService.renameConfig(oldname, newname);
    } catch (err) {
      console.error("Failed to rename config:", err);
    }
  }

  async function onDelete(name: string) {
    try {
      await configService.deleteConfig(name);
    } catch (err) {
      console.error("Failed to delete config:", err);
    }
  }
</script>

<div class="configs">
  {#if $configsError}
    <p class="error">{$configsError}</p>
  {/if}

  {#if $configsLoading && $configNames.length === 0}
    <p>Waiting for Pipo to send configs...</p>
  {:else if $configNames.length > 0}
    <Tabs
      items={$configNames}
      active={$activeConfigName}
      {onDelete}
      {onCreate}
      {onClick}
      {onRename}
    >
      {#if $currentConfig}
        <Config />
      {/if}
    </Tabs>
  {/if}
</div>

<style>
  .configs {
    width: 100%;
  }
</style>
