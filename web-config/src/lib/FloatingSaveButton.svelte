<script lang="ts">
  import { pipoio } from "../pipoio";
  import { activeConfigName } from "../services/config";
  import { get } from "svelte/store";

  export let config: any;
  export let show: boolean = false;
  export let onSaveSuccess: () => void = () => {};

  let savingStatus: "none" | "loading" | "success" | "error" = "none";

  async function handleSave() {
    console.log("Saving config...");
    savingStatus = "loading";

    const name = get(activeConfigName);
    const blob = new Blob([JSON.stringify(config)], {
      type: "application/json",
    });
    const formData = new FormData();
    formData.append("file", blob, name);

    try {
      await Promise.all([
        new Promise((resolve) => setTimeout(resolve, 1000)),
        pipoio.request({
          method: "post",
          url: "/save",
          data: formData,
          headers: { "Content-Type": "multipart/form-data" },
        }),
      ]);

      console.log("Config saved successfully");
      savingStatus = "success";

      // Call success callback after a brief delay
      setTimeout(() => {
        onSaveSuccess();
        savingStatus = "none";
      }, 1500);
    } catch (error) {
      console.error("Failed to save config:", error);
      savingStatus = "error";

      setTimeout(() => {
        savingStatus = "none";
      }, 2000);
    }
  }
</script>

{#if show}
  <div class="floating-save-container" class:show>
    <button
      class="floating-save-btn"
      class:loading={savingStatus === "loading"}
      class:success={savingStatus === "success"}
      class:error={savingStatus === "error"}
      on:click={handleSave}
      disabled={savingStatus === "loading"}
    >
      {#if savingStatus === "loading"}
        <span class="spinner"></span>
        Saving...
      {:else if savingStatus === "success"}
        ✓ Saved!
      {:else if savingStatus === "error"}
        ✗ Error
      {:else}
        Save Changes
      {/if}
    </button>
  </div>
{/if}

<style>
  .floating-save-container {
    position: fixed;
    bottom: 24px;
    right: 24px;
    z-index: 1000;
    opacity: 0;
    transform: translateY(20px);
    transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
    pointer-events: none;
  }

  .floating-save-container.show {
    opacity: 1;
    transform: translateY(0);
    pointer-events: auto;
  }

  .floating-save-btn {
    display: flex;
    align-items: center;
    gap: 8px;
    padding: 14px 24px;
    background-color: var(--main);
    color: var(--bg-primary);
    border: none;
    border-radius: 24px;
    font-size: 16px;
    font-weight: 600;
    cursor: pointer;
    box-shadow: 0 4px 12px rgba(0, 0, 0, 0.3);
    transition: all 0.2s ease;
    min-width: 150px;
    justify-content: center;
  }

  .floating-save-btn:hover:not(:disabled) {
    transform: translateY(-2px);
    box-shadow: 0 6px 16px rgba(0, 0, 0, 0.4);
    background-color: var(--main-lighter);
  }

  .floating-save-btn:active:not(:disabled) {
    transform: translateY(0);
  }

  .floating-save-btn:disabled {
    cursor: wait;
    opacity: 0.8;
  }

  .floating-save-btn.success {
    background-color: var(--green);
    color: white;
  }

  .floating-save-btn.error {
    background-color: var(--red);
    color: white;
  }

  .spinner {
    width: 16px;
    height: 16px;
    border: 2px solid var(--bg-primary);
    border-bottom-color: transparent;
    border-radius: 50%;
    display: inline-block;
    box-sizing: border-box;
    animation: rotation 1s linear infinite;
  }

  @keyframes rotation {
    0% {
      transform: rotate(0deg);
    }
    100% {
      transform: rotate(360deg);
    }
  }
</style>
