<script lang="ts">
  import { get } from "svelte/store";
  import Modal from "../modal.svelte";
  import ConfigNameDialog from "./ConfigNameDialog.svelte";
  import { addToast } from "../toast";
  import { pipoio } from "../../pipoio";
  import {
    configService,
    configMetas,
    currentConfig,
    hasUnsavedChanges,
    activeConfigName,
    originalConfig,
  } from "../../services/config";
  import type { ConfigMeta, PipoConfig, PipoTypes } from "../../types";
  import { Copy, Pencil, Trash2, Upload } from "lucide-svelte";

  export let open = false;

  const MAX_CONFIGS = 8;

  let nameDialogOpen = false;
  let nameDialogTitle = "";
  let nameDialogInitial = "";
  let nameDialogConfirmLabel = "";
  let nameDialogAction: "new" | "duplicate" | "rename" = "new";
  let nameDialogTarget = "";
  let loading = false;
  let confirmOpen = false;
  let confirmMessage = "";
  let confirmAction: (() => Promise<void>) | null = null;
  let confirmAltAction: (() => Promise<void>) | null = null;
  let confirmAltLabel = "";

  $: metas = $configMetas;
  $: names = metas.map((m) => m.name);
  $: atLimit = metas.length >= MAX_CONFIGS;

  function showConfirm(
    message: string,
    action: () => Promise<void>,
    alt?: { label: string; action: () => Promise<void> },
  ) {
    confirmMessage = message;
    confirmAction = action;
    confirmAltAction = alt?.action ?? null;
    confirmAltLabel = alt?.label ?? "";
    confirmOpen = true;
  }

  async function handleConfirmOk() {
    confirmOpen = false;
    if (confirmAction) {
      await confirmAction();
      confirmAction = null;
    }
    confirmAltAction = null;
  }

  async function handleConfirmAlt() {
    confirmOpen = false;
    if (confirmAltAction) {
      await confirmAltAction();
      confirmAltAction = null;
    }
    confirmAction = null;
  }

  function handleConfirmCancel() {
    confirmOpen = false;
    confirmAction = null;
    confirmAltAction = null;
  }

  // ---- NEW CONFIG ----
  function handleNew() {
    nameDialogAction = "new";
    nameDialogTitle = "New Config";
    nameDialogInitial = "";
    nameDialogConfirmLabel = "Create";
    nameDialogOpen = true;
  }

  // ---- DUPLICATE ----
  function handleDuplicate(meta: ConfigMeta) {
    nameDialogAction = "duplicate";
    nameDialogTarget = meta.name;
    nameDialogTitle = "Duplicate Config";
    nameDialogInitial = "";
    nameDialogConfirmLabel = "Duplicate";
    nameDialogOpen = true;
  }

  // ---- RENAME ----
  function handleRename(meta: ConfigMeta) {
    nameDialogAction = "rename";
    nameDialogTarget = meta.name;
    nameDialogTitle = "Rename Config";
    nameDialogInitial = meta.name;
    nameDialogConfirmLabel = "Rename";
    nameDialogOpen = true;
  }

  // ---- DELETE ----
  function handleDelete(meta: ConfigMeta) {
    showConfirm(`Delete "${meta.name}"? This cannot be undone.`, async () => {
      loading = true;
      try {
        await configService.deleteConfig(meta.name);
        await configService.fetchConfigNames();
        addToast({ type: "info", message: "Config deleted", timeout: 2000 });
      } catch (err) {
        addToast({
          type: "error",
          message: "Failed to delete config",
          timeout: 4000,
        });
      } finally {
        loading = false;
      }
    });
  }

  // ---- LOAD ----
  async function handleLoad(meta: ConfigMeta) {
    // 1. Check unsaved changes
    if (get(hasUnsavedChanges)) {
      showConfirm(
        "You have unsaved changes. Discard and load this config?",
        () => doLoad(meta),
      );
      return;
    }
    await doLoad(meta);
  }

  async function doLoad(meta: ConfigMeta) {
    loading = true;
    try {
      // Fetch target config content
      const targetConfig = await configService.fetchConfig(meta.name);
      if (!targetConfig) {
        addToast({
          type: "error",
          message: "Failed to load config",
          timeout: 4000,
        });
        loading = false;
        return;
      }

      const current = get(currentConfig);
      let willReboot = false;
      let warnings: string[] = [];
      let oscConflict = false;

      if (current) {
        // Check mode change
        const currentIsMidi = current.general.MidiEnabled;
        const targetIsMidi = targetConfig.general.MidiEnabled;
        const currentIsOsc = current.general.OSC_ENA;
        const targetIsOsc = targetConfig.general.OSC_ENA;

        if (currentIsMidi !== targetIsMidi || currentIsOsc !== targetIsOsc) {
          willReboot = true;
          warnings.push("Output mode will change (MIDI ↔ OSC) → reboot");
        }

        // Preserve PipoName — switching configs should not change device name
        targetConfig.general.PipoName = current.general.PipoName;

        // Check OSC IP/port — only when both configs are OSC and mode isn't changing
        const bothOsc = targetConfig.general.OSC_ENA && current.general.OSC_ENA;
        const modeChanging =
          currentIsMidi !== targetIsMidi || currentIsOsc !== targetIsOsc;

        if (bothOsc && !modeChanging) {
          const ipChanged =
            current.general.OSC_IP !== targetConfig.general.OSC_IP;
          const portChanged =
            current.general.OSC_PORT !== targetConfig.general.OSC_PORT;

          if (ipChanged || portChanged) {
            oscConflict = true;
            warnings.push(
              `OSC destination differs:\n  Target: ${targetConfig.general.OSC_IP}:${targetConfig.general.OSC_PORT}\n  Current: ${current.general.OSC_IP}:${current.general.OSC_PORT}`,
            );
          }
        }
      }

      // No conflicts — load directly
      if (warnings.length === 0) {
        await activateConfig(meta.name, false, []);
        return;
      }

      // Show all conflicts in one dialog
      loading = false;
      let message = `Loading "${meta.name}":\n\n• ${warnings.join("\n• ")}`;
      if (willReboot) {
        message += "\n\nThe device will reboot.";
      }
      message += "\n\nContinue?";

      showConfirm(
        message,
        async () => {
          // "Keep Current" — patch target with current OSC settings
          if (oscConflict) {
            const cur = get(currentConfig)!;
            targetConfig.general.OSC_IP = cur.general.OSC_IP;
            targetConfig.general.OSC_PORT = cur.general.OSC_PORT;
            await configService.saveConfig(
              targetConfig as PipoConfig<PipoTypes>,
              meta.name,
            );
          }
          await activateConfig(meta.name, willReboot, warnings);
        },
        oscConflict
          ? {
              label: "Use Target",
              action: async () => {
                // Load as-is, don't patch OSC
                await activateConfig(meta.name, willReboot, warnings);
              },
            }
          : undefined,
      );
    } catch (err) {
      addToast({
        type: "error",
        message: "Failed to load config",
        timeout: 4000,
      });
    } finally {
      loading = false;
    }
  }

  async function activateConfig(
    name: string,
    reboot: boolean,
    reasons: string[],
  ) {
    loading = true;
    try {
      await configService.setActiveConfig(name);
      await configService.fetchConfigNames();

      if (reboot) {
        addToast({
          type: "error",
          message:
            "Pipo is rebooting... Please reload the page in a few seconds. Make sure WiFi is reconnected.",
          timeout: 10000,
        });
        pipoio
          .get("/reboot")
          .then(() => console.log("Rebooting device..."))
          .catch((err: unknown) => console.error("Failed to reboot:", err));
      } else {
        addToast({
          type: "info",
          message: `Config "${name}" loaded`,
          timeout: 2000,
        });
      }
      open = false;
    } catch (err) {
      addToast({
        type: "error",
        message: "Failed to activate config",
        timeout: 4000,
      });
    } finally {
      loading = false;
    }
  }

  // ---- NAME DIALOG HANDLER ----
  async function handleNameConfirm(e: CustomEvent<string>) {
    const newName = e.detail;
    loading = true;
    try {
      if (nameDialogAction === "new") {
        await configService.createConfig(newName);
        addToast({
          type: "info",
          message: `Config "${newName}" created`,
          timeout: 2000,
        });
      } else if (nameDialogAction === "duplicate") {
        await configService.duplicateConfig(nameDialogTarget, newName);
        addToast({
          type: "info",
          message: `Config duplicated as "${newName}"`,
          timeout: 2000,
        });
      } else if (nameDialogAction === "rename") {
        await configService.renameConfig(nameDialogTarget, newName);
        addToast({
          type: "info",
          message: `Config renamed to "${newName}"`,
          timeout: 2000,
        });
      }
      await configService.fetchConfigNames();
    } catch (err: unknown) {
      const msg = err instanceof Error ? err.message : "Operation failed";
      addToast({ type: "error", message: msg, timeout: 4000 });
    } finally {
      loading = false;
    }
  }
</script>

<Modal bind:open>
  <div class="config-manager">
    <h3>Configurations</h3>
    <span class="counter">{metas.length}/{MAX_CONFIGS}</span>

    <div class="config-list scroll">
      {#each metas as meta (meta.name)}
        <div class="config-row" class:active={meta.active}>
          <div class="config-info">
            <span class="config-name">{meta.name}</span>
            {#if meta.active}
              <span class="active-badge">Active</span>
            {/if}
          </div>
          <div class="config-actions">
            {#if !meta.active}
              <button
                class="icon-btn"
                title="Load"
                on:click={() => handleLoad(meta)}
                disabled={loading}
              >
                <Upload size={14} />
              </button>
            {/if}
            <button
              class="icon-btn"
              title="Duplicate"
              on:click={() => handleDuplicate(meta)}
              disabled={loading || atLimit}
            >
              <Copy size={14} />
            </button>
            <button
              class="icon-btn"
              title="Rename"
              on:click={() => handleRename(meta)}
              disabled={loading}
            >
              <Pencil size={14} />
            </button>
            {#if !meta.active}
              <button
                class="icon-btn delete"
                title="Delete"
                on:click={() => handleDelete(meta)}
                disabled={loading}
              >
                <Trash2 size={14} />
              </button>
            {/if}
          </div>
        </div>
      {/each}
    </div>

    <button
      class="primary new-btn"
      on:click={handleNew}
      disabled={loading || atLimit}
    >
      + New Config
    </button>
  </div>
</Modal>

<!-- Name dialog for create / duplicate / rename -->
<ConfigNameDialog
  bind:open={nameDialogOpen}
  title={nameDialogTitle}
  initialValue={nameDialogInitial}
  existingNames={names}
  confirmLabel={nameDialogConfirmLabel}
  on:confirm={handleNameConfirm}
/>

<!-- Confirmation dialog -->
{#if confirmOpen}
  <div
    class="confirm-overlay"
    on:click={handleConfirmCancel}
    on:keyup={() => {}}
  >
    <div class="confirm-box" on:click|stopPropagation on:keyup={() => {}}>
      <p class="confirm-message">{confirmMessage}</p>
      <div class="confirm-actions">
        <button class="secondary" on:click={handleConfirmCancel}>Cancel</button>
        {#if confirmAltAction}
          <button class="secondary" on:click={handleConfirmAlt}>{confirmAltLabel}</button>
        {/if}
        <button class="primary" on:click={handleConfirmOk}>
          {confirmAltAction ? 'Keep Current' : 'Continue'}
        </button>
      </div>
    </div>
  </div>
{/if}

<style>
  .config-manager {
    padding: 16px;
    display: flex;
    flex-direction: column;
    gap: 12px;
    min-width: 280px;
  }
  h3 {
    margin: 0;
    font-size: 16px;
    color: var(--main);
  }
  .counter {
    font-size: 12px;
    color: var(--text-color-secondary);
    margin-top: -8px;
  }
  .config-list {
    display: flex;
    flex-direction: column;
    gap: 6px;
    max-height: 320px;
    overflow-y: auto;
  }
  .config-row {
    display: flex;
    align-items: center;
    justify-content: space-between;
    padding: 8px 10px;
    border-radius: 4px;
    background-color: var(--bg-secondary);
    gap: 8px;
  }
  .config-row.active {
    border: 1px solid var(--main);
  }
  .config-info {
    display: flex;
    align-items: center;
    gap: 6px;
    flex-shrink: 1;
    min-width: 0;
  }
  .config-name {
    font-size: 13px;
    font-weight: 600;
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
  }
  .active-badge {
    font-size: 10px;
    padding: 1px 5px;
    border-radius: 3px;
    background-color: var(--main);
    color: var(--bg-primary);
    font-weight: 700;
    flex-shrink: 0;
  }
  .config-actions {
    display: flex;
    gap: 4px;
    flex-shrink: 0;
  }
  .icon-btn {
    width: 26px;
    height: 26px;
    min-width: 26px;
    display: flex;
    align-items: center;
    justify-content: center;
    border: none;
    border-radius: 4px;
    background: transparent;
    color: var(--grey);
    cursor: pointer;
    padding: 0;
  }
  .icon-btn:hover:not(:disabled) {
    background-color: var(--bg-tertiary);
    color: var(--text-color);
  }
  .icon-btn.delete:hover:not(:disabled) {
    color: var(--red);
  }
  .icon-btn:disabled {
    opacity: 0.3;
    cursor: not-allowed;
  }
  .new-btn {
    width: 100%;
    margin-top: 4px;
  }
  .new-btn:disabled {
    opacity: 0.5;
    cursor: not-allowed;
  }
  /* Confirmation overlay */
  .confirm-overlay {
    position: fixed;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    background: rgba(0, 0, 0, 0.6);
    z-index: 30;
    display: flex;
    align-items: center;
    justify-content: center;
    backdrop-filter: blur(5px);
  }
  .confirm-box {
    background-color: var(--bg-network);
    border-radius: 8px;
    padding: 20px;
    max-width: 340px;
    width: calc(100vw - 40px);
  }
  .confirm-message {
    margin: 0 0 16px 0;
    font-size: 13px;
    line-height: 1.5;
    white-space: pre-line;
    color: var(--text-color);
  }
  .confirm-actions {
    display: flex;
    gap: 8px;
    justify-content: flex-end;
  }
</style>
