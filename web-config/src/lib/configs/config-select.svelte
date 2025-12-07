<script lang="ts">
  import Select from "svelte-select";
  import LoadingButton from "../form/LoadingButton.svelte";
  import Modal from "../modal.svelte";
  import {
    configService,
    configNames,
    activeConfigName,
    configsLoading,
    currentConfig,
  } from "../../services/config";
  import { Plus, Trash2, CopyPlus } from "lucide-svelte";

  let showRenameModal = false;
  let showCopyModal = false;
  let showDeleteModal = false;
  let showCreateModal = false;
  let newname = "";
  let copyName = "";
  let createName = "";
  let deleting = false;
  let creating = false;
  let duplicating = false;
  let touchTimer: number | null = null;
  let touchStartTime = 0;

  $: selectItems = $configNames.map((item) => ({ value: item, label: item }));
  $: selectedItem = $activeConfigName
    ? { value: $activeConfigName, label: $activeConfigName }
    : null;

  async function handleSelect(event: CustomEvent) {
    const selected = event.detail;
    if (selected && selected.value !== $activeConfigName) {
      await configService.setActiveConfig(selected.value);
    }
  }

  function startRename() {
    newname = $activeConfigName;
    showRenameModal = true;
  }

  async function finishRename() {
    if (newname.trim() && newname.trim() !== $activeConfigName) {
      await configService.renameConfig($activeConfigName, newname.trim());
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
    copyName = `${$activeConfigName} - Copy`;
    showCopyModal = true;
  }

  async function finishCopy() {
    if (copyName.trim() && $currentConfig) {
      duplicating = true;
      try {
        await configService.copyConfig(copyName.trim(), $currentConfig);
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
    if (createName.trim()) {
      creating = true;
      try {
        await configService.createConfig(createName.trim());
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
      <span> Current Config </span>
      <div
        on:dblclick={handleSelectDoubleClick}
        on:touchstart={handleTouchStart}
        on:touchend={handleTouchEnd}
        on:touchmove={handleTouchMove}
      >
        <Select
          --height="18px"
          --max-height="18px"
          --width="134px"
          --padding="0"
          --value-container-padding="0"
          --selected-item-padding="0 0 0 10px"
          --selected-item-color="var(--bg-primary)"
          --font-size="16px"
          --color="var(--bg-primary)"
          --item-color="var(--bg-primary)"
          --item-bg="var(--bg-secondary)"
          --item-is-active-color="var(--main)"
          --item-is-active-bg="var(--bg-secondary)"
          --item-hover-color="var(--text-color)"
          --item-hover-bg="var(--bg-secondary)"
          --input-color="var(--text-color)"
          items={selectItems}
          value={selectedItem}
          on:change={handleSelect}
          clearable={false}
          searchable={false}
          disabled={$configsLoading}
        />
      </div>
    </div>

    <div class="actions">
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
    </div>
  </div>
</section>

<Modal bind:open={showRenameModal}>
  <div class="modal-content">
    <h3>Rename Configuration</h3>
    <input
      type="text"
      bind:value={newname}
      on:keyup={onRenameKeyup}
      placeholder="Enter new name"
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
      on:keyup={onCopyKeyup}
      placeholder="Enter copy name"
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
      on:keyup={onCreateKeyup}
      placeholder="Enter configuration name"
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

<style scoped>
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
    width: 19px;
    height: 19px;
    min-width: 19px;
    min-height: 19px;
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
    border: 1px solid var(--border-color, #ccc);
    border-radius: 4px;
    background-color: var(--bg-primary);
    color: var(--text-color);
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
</style>
