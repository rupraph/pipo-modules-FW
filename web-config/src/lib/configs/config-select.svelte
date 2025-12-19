<script lang="ts">
  import Select from "svelte-select";
  import LoadingButton from "../form/LoadingButton.svelte";
  import Modal from "../modal.svelte";
  import InfoModal from "../InfoModal.svelte";
  import {
    configService,
    configNames,
    activeConfigName,
    configsLoading,
    currentConfig,
  } from "../../services/config";
  import { Plus, Trash2, CopyPlus } from "lucide-svelte";
  import { schema } from "../../schema";
  import { pipoio } from "../../pipoio";
  import { addToast } from "../toast";

  let showRenameModal = false;
  let showCopyModal = false;
  let showDeleteModal = false;
  let showCreateModal = false;
  let showApplyConfigModal = false;
  let pendingConfigName = "";
  let pendingConfigData: any = null;
  let willRebootForMode = false;
  let willRebootForName = false;
  let loadingPendingConfig = false;
  let applyingConfig = false;
  let newname = "";
  let copyName = "";
  let createName = "";
  let deleting = false;
  let creating = false;
  let duplicating = false;
  let touchTimer: number | null = null;
  let touchStartTime = 0;

  let config_name_max_length = 16;

  function filterConfigName(value: string): string {
    // Only allow alphanumeric characters and dashes
    return value.replace(/[^a-zA-Z0-9-]/g, "");
  }

  function handleRenameInput(e: Event) {
    const input = e.target as HTMLInputElement;
    let filtered = filterConfigName(input.value);

    // Apply max length limit
    if (filtered.length > config_name_max_length) {
      filtered = filtered.slice(0, config_name_max_length);
      input.value = filtered;
    }

    newname = filtered;
  }

  function handleCopyInput(e: Event) {
    const input = e.target as HTMLInputElement;
    let filtered = filterConfigName(input.value);

    // Apply max length limit
    if (filtered.length > config_name_max_length) {
      filtered = filtered.slice(0, config_name_max_length);
      input.value = filtered;
    }

    copyName = filtered;
  }

  function handleCreateInput(e: Event) {
    const input = e.target as HTMLInputElement;
    let filtered = filterConfigName(input.value);

    // Apply max length limit
    if (filtered.length > config_name_max_length) {
      filtered = filtered.slice(0, config_name_max_length);
      input.value = filtered;
    }

    createName = filtered;
  }

  $: selectItems = $configNames.map((item) => ({ value: item, label: item }));
  $: selectedItem = $activeConfigName
    ? { value: $activeConfigName, label: $activeConfigName }
    : null;
  $: willReboot = willRebootForMode || willRebootForName;

  async function handleSelect(event: CustomEvent) {
    const selected = event.detail;
    if (selected && selected.value !== $activeConfigName) {
      // Store pending config and show confirmation modal
      pendingConfigName = selected.value;
      loadingPendingConfig = true;
      showApplyConfigModal = true;

      try {
        // Fetch the pending config to check for mode/name changes
        pendingConfigData = await configService.fetchConfig(selected.value);

        if (pendingConfigData && $currentConfig) {
          // Check if mode will change
          const currentMode = $currentConfig.general.MidiEnabled
            ? "MIDI"
            : "OSC";
          const pendingMode = pendingConfigData.general.MidiEnabled
            ? "MIDI"
            : "OSC";
          willRebootForMode = currentMode !== pendingMode;

          // Check if PipoName will change
          willRebootForName =
            $currentConfig.general.PipoName !==
            pendingConfigData.general.PipoName;
        }
      } catch (err) {
        console.error("Error fetching pending config:", err);
        showApplyConfigModal = false;
      } finally {
        loadingPendingConfig = false;
      }
    }
  }

  async function confirmApplyConfig() {
    if (!pendingConfigName) return;

    const needsReboot = willRebootForMode || willRebootForName;
    applyingConfig = true;
    try {
      await configService.setActiveConfig(pendingConfigName);
      showApplyConfigModal = false;
      pendingConfigName = "";
      pendingConfigData = null;

      // If reboot is needed, show toast and trigger reboot
      if (needsReboot) {
        let rebootReason = "";
        if (willRebootForMode && willRebootForName) {
          rebootReason = "MIDI/OSC mode and Pipo name changes";
        } else if (willRebootForMode) {
          rebootReason = "MIDI/OSC mode change";
        } else if (willRebootForName) {
          rebootReason = "Pipo name change";
        }

        addToast({
          type: "error",
          message: `Pipo will reboot to take into account the ${rebootReason}. Please reload the page in a few seconds (Make sure Wifi is reconnected)`,
          timeout: 10000,
        });

        pipoio
          .get("/reboot")
          .then(() => {
            console.log("Rebooting device...");
          })
          .catch((err) => {
            console.error("Failed to reboot:", err);
          });
      }

      willRebootForMode = false;
      willRebootForName = false;
    } catch (err) {
      console.error("Error applying config:", err);
    } finally {
      applyingConfig = false;
    }
  }

  function cancelApplyConfig() {
    showApplyConfigModal = false;
    pendingConfigName = "";
    pendingConfigData = null;
    willRebootForMode = false;
    willRebootForName = false;
    // Reset the select to current active config
    selectedItem = $activeConfigName
      ? { value: $activeConfigName, label: $activeConfigName }
      : null;
  }

  function startRename() {
    newname = $activeConfigName;
    showRenameModal = true;
  }

  async function finishRename() {
    const trimmedName = newname.trim();
    if (trimmedName && trimmedName !== $activeConfigName) {
      await configService.renameConfig($activeConfigName, trimmedName);
    }
    showRenameModal = false;
    newname = "";
  }

  function onRenameKeyup(e: KeyboardEvent) {
    if (e.key === "Enter") {
      finishRename();
    } else if (e.key === "Escape") {
      showRenameModal = false;
      newname = "";
    }
  }

  function handleSelectDoubleClick() {
    if (!$configsLoading && $activeConfigName) {
      startRename();
    }
  }

  function handleTouchStart() {
    if ($configsLoading || !$activeConfigName) return;

    touchStartTime = Date.now();
    touchTimer = window.setTimeout(() => {
      startRename();
      touchTimer = null;
    }, 500); // 500ms for long press
  }

  function handleTouchEnd() {
    if (touchTimer !== null) {
      clearTimeout(touchTimer);
      touchTimer = null;
    }
  }

  function handleTouchMove() {
    // Cancel long press if user moves finger
    if (touchTimer !== null) {
      clearTimeout(touchTimer);
      touchTimer = null;
    }
  }

  function startDelete() {
    showDeleteModal = true;
  }

  async function confirmDelete() {
    deleting = true;
    try {
      await configService.deleteConfig($activeConfigName);
      showDeleteModal = false;
    } finally {
      deleting = false;
    }
  }

  function startCopy() {
    const suffix = "-CPY";
    const maxBaseLength = config_name_max_length - suffix.length;
    const baseName = $activeConfigName.slice(0, maxBaseLength);
    copyName = `${baseName}${suffix}`;
    showCopyModal = true;
  }

  async function finishCopy() {
    const trimmedName = copyName.trim();
    if (trimmedName && $currentConfig) {
      duplicating = true;
      try {
        await configService.copyConfig(trimmedName, $currentConfig);
      } finally {
        duplicating = false;
        showCopyModal = false;
        copyName = "";
      }
    }
  }

  function onCopyKeyup(e: KeyboardEvent) {
    if (e.key === "Enter") {
      finishCopy();
    } else if (e.key === "Escape") {
      showCopyModal = false;
      copyName = "";
    }
  }
  function startCreate() {
    createName = "";
    showCreateModal = true;
  }

  async function finishCreate() {
    const trimmedName = createName.trim();
    if (trimmedName) {
      creating = true;
      try {
        await configService.createConfig(trimmedName);
        showCreateModal = false;
        createName = "";
      } finally {
        creating = false;
      }
    }
  }

  function onCreateKeyup(e: KeyboardEvent) {
    if (e.key === "Enter") {
      finishCreate();
    } else if (e.key === "Escape") {
      showCreateModal = false;
      createName = "";
    }
  }
</script>

<section>
  <div class="row">
    <div class="left">
      <span> Config Select </span>
      <InfoModal>
        <p>
          Select in the list the active configuration to use. To manage
          configurations, you can:
        </p>
        <ul
          style="justify-content: flex-start; text-align: left; margin-top: 0.5em;"
        >
          <li>
            Clone the current configuration using
            <CopyPlus
              size={16}
              style="display: inline; vertical-align: middle;"
            />
          </li>
          <li>
            Create a new configuration from the factory default with <Plus
              size={16}
              style="display: inline; vertical-align: middle;"
            />
          </li>
          <li>
            Delete the current configuration using
            <Trash2
              size={16}
              style="display: inline; vertical-align: middle;"
            />
          </li>
        </ul>
      </InfoModal>
    </div>
    <div
      class="select-container"
      on:dblclick={handleSelectDoubleClick}
      on:touchstart={handleTouchStart}
      on:touchend={handleTouchEnd}
      on:touchmove={handleTouchMove}
    >
      <Select
        --height="28px"
        --max-height="28px"
        --list-width="200px"
        --padding="0"
        --value-container-padding="0"
        --selected-item-padding="0 3px 0 10px"
        --selected-item-color="var(--bg-primary)"
        --font-size="16px"
        --color="var(--bg-primary)"
        --item-color="var(--text-color)"
        --item-bg="var(--bg-tertiary)"
        --item-is-active-color="var(--main-lighter)"
        --item-is-active-bg="var(--bg-secondary)"
        --input-color="var(--text-color-secondary)"
        --list-background="var(--bg-tertiary)"
        --item-hover-bg="var(--bg-tertiary)"
        --list-border="2px solid var(--text-color)"
        items={selectItems}
        value={selectedItem}
        on:change={handleSelect}
        clearable={false}
        searchable={false}
        disabled={$configsLoading}
      />
    </div>

    <div class="actions">
      <LoadingButton
        onClick={startCreate}
        class="action-btn new-btn"
        loading={creating}
        disabled={$configsLoading}
        title="New Config"
        width="1.5em"
      >
        <Plus />
      </LoadingButton>
      <LoadingButton
        onClick={startCopy}
        class="action-btn "
        loading={duplicating}
        disabled={$configsLoading}
        title="Duplicate"
        width="1.5em"
      >
        <CopyPlus />
      </LoadingButton>
      <LoadingButton
        onClick={startDelete}
        class="action-btn delete-btn"
        loading={deleting}
        disabled={$configsLoading}
        title="Delete"
        width="1.5em"
      >
        <Trash2 />
      </LoadingButton>
    </div>
  </div>
</section>

<Modal bind:open={showRenameModal}>
  <div class="modal-content">
    <h3>Rename Configuration</h3>
    <input
      type="text"
      bind:value={newname}
      on:input={handleRenameInput}
      on:keyup={onRenameKeyup}
      placeholder="Enter new name (letters, numbers, dashes)"
      autofocus
    />
    <div class="modal-buttons">
      <button
        on:click={() => {
          showRenameModal = false;
          newname = "";
        }}>Cancel</button
      >
      <button on:click={finishRename} disabled={!newname.trim()}>Rename</button>
    </div>
  </div>
</Modal>

<Modal bind:open={showCopyModal}>
  <div class="modal-content">
    <h3>Copy Configuration</h3>
    <input
      type="text"
      bind:value={copyName}
      on:input={handleCopyInput}
      on:keyup={onCopyKeyup}
      placeholder="Enter copy name (letters, numbers, dashes)"
      autofocus
    />
    <div class="modal-buttons">
      <button
        on:click={() => {
          showCopyModal = false;
          copyName = "";
        }}>Cancel</button
      >
      <button on:click={finishCopy} disabled={!copyName.trim() || duplicating}>
        {duplicating ? "Copying..." : "Copy"}
      </button>
    </div>
  </div>
</Modal>

<Modal bind:open={showDeleteModal}>
  <div class="modal-content">
    <h3>Delete Configuration</h3>
    <p>
      Are you sure you want to delete the configuration "{$activeConfigName}"?
    </p>
    <p class="warning">This action cannot be undone.</p>
    <div class="modal-buttons">
      <button
        on:click={() => {
          showDeleteModal = false;
        }}>Cancel</button
      >
      <button
        class="delete-confirm-btn"
        on:click={confirmDelete}
        disabled={deleting}
      >
        {deleting ? "Deleting..." : "Delete"}
      </button>
    </div>
  </div>
</Modal>

<Modal bind:open={showCreateModal}>
  <div class="modal-content">
    <h3>Create New Configuration</h3>
    <input
      type="text"
      bind:value={createName}
      on:input={handleCreateInput}
      on:keyup={onCreateKeyup}
      placeholder="New name (letters, numbers, dashes)"
      autofocus
    />
    <div class="modal-buttons">
      <button
        on:click={() => {
          showCreateModal = false;
          createName = "";
        }}>Cancel</button
      >
      <button on:click={finishCreate} disabled={!createName.trim() || creating}>
        {creating ? "Creating..." : "Create"}
      </button>
    </div>
  </div>
</Modal>

<Modal bind:open={showApplyConfigModal}>
  <div class="modal-content">
    <h3>Apply Configuration</h3>
    {#if loadingPendingConfig}
      <p>Loading configuration...</p>
    {:else}
      <p>
        Do you want to apply the configuration <strong
          >"{pendingConfigName}"</strong
        >?
      </p>
      {#if willReboot}
        <div class="warning-box">
          <p style="margin: 0; font-weight: 600; color: var(--red);">
            This will trigger a reboot of the board.
          </p>
          <p style="margin: 0.5em 0 0 0; font-size: 14px;">
            {#if willRebootForMode && willRebootForName}
              This config has a different MIDI/OSC mode and Pipo name.
            {:else if willRebootForMode}
              This config has a different MIDI/OSC mode.
            {:else if willRebootForName}
              This config has a different Pipo name.
            {/if}
          </p>
        </div>
      {/if}
    {/if}
    <div class="modal-buttons">
      <button on:click={cancelApplyConfig} disabled={applyingConfig}>
        Cancel
      </button>
      <button
        on:click={confirmApplyConfig}
        disabled={loadingPendingConfig || applyingConfig}
        class:reboot-btn={willReboot}
      >
        {#if applyingConfig}
          Applying...
        {:else if willReboot}
          Apply and Reboot
        {:else}
          Apply
        {/if}
      </button>
    </div>
  </div>
</Modal>

<style scoped>
  .left {
    flex-wrap: wrap;
    gap: 4px;
    align-items: center;
  }

  .select-container {
    flex: 1;
    min-width: 130px;
    max-width: 200px;
    margin: 5px;
  }

  .row :global(.svelte-select) {
    width: 100% !important;
  }

  .row :global(.svelte-select-list) {
    width: 200px !important;
    min-width: 160px !important;
    left: 50% !important;
    transform: translateX(-50%) !important;
  }

  .row :global(.selected-item) {
    line-height: var(--font-size);
  }
  .row span {
    font-size: 14px;
    line-height: 14px;
  }

  .actions {
    display: flex;
    gap: 0.5em;
    align-items: center;
  }

  .actions :global(.action-btn) {
    width: 25px;
    height: 25px;
    min-width: 25px;
    min-height: 25px;
    padding: 0;
    display: flex;
    align-items: center;
    justify-content: center;
  }

  .actions :global(.action-btn svg) {
    width: 14px;
    height: 14px;
  }

  :global(.action-btn.delete-btn) {
    background-color: transparent;
    border-color: var(--red);
    color: var(--red);
  }

  :global(.action-btn.delete-btn:hover:not(:disabled)) {
    background-color: var(--red);
    color: var(--grey);
  }

  .modal-content {
    padding: 2em;
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 1.5em;
  }

  .modal-content h3 {
    margin: 0;
    font-size: 1.2em;
  }

  .modal-content p {
    margin: 0;
    line-height: 1.5;
  }

  .modal-content .warning {
    color: var(--red, #ff4444);
    font-weight: bold;
    font-size: 0.9em;
  }

  .modal-content input {
    padding: 0.5em;
    font-size: 1em;
    border: 1px solid var(--main);
    border-radius: 4px;
    background-color: var(--bg-primary);
    color: var(--text-color);
    max-width: 220px;
  }

  .modal-buttons {
    display: flex;
    gap: 1em;
    justify-content: flex-end;
  }

  .modal-buttons button {
    padding: 0.5em 1em;
    font-size: 1em;
    border-radius: 4px;
    cursor: pointer;
    border: 1px solid var(--border-color, #ccc);
    background-color: var(--bg-primary);
    color: var(--text-color);
  }

  .modal-buttons button:hover:not(:disabled) {
    background-color: var(--bg-secondary);
  }

  .modal-buttons button:disabled {
    opacity: 0.5;
    cursor: not-allowed;
  }

  .modal-buttons .delete-confirm-btn {
    background-color: var(--red, #ff4444);
    color: white;
    border-color: var(--red, #ff4444);
  }

  .modal-buttons .delete-confirm-btn:hover:not(:disabled) {
    background-color: #cc0000;
    border-color: #cc0000;
  }

  .warning-box {
    background-color: rgba(255, 68, 68, 0.1);
    border: 2px solid var(--red, #ff4444);
    border-radius: 8px;
    padding: 1em;
    width: 100%;
    box-sizing: border-box;
  }

  .modal-buttons .reboot-btn {
    background-color: var(--red, #ff4444);
    color: white;
    border-color: var(--red, #ff4444);
  }

  .modal-buttons .reboot-btn:hover:not(:disabled) {
    background-color: #cc0000;
    border-color: #cc0000;
  }
</style>
