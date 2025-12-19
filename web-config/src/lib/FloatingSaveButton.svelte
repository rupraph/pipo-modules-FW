<script lang="ts">
  import { pipoio } from "../pipoio";
  import {
    activeConfigName,
    modeWillChange,
    pipoNameWillChange,
  } from "../services/config";
  import { get } from "svelte/store";
  import { onMount, onDestroy } from "svelte";
  import { addToast } from "./toast";
  import {
    validateConfigForSave,
    getValidationErrorSummary,
  } from "../utils/config-validator";

  export let config: any;
  export let show: boolean = false;
  export let onSaveSuccess: () => void = () => {};

  let savingStatus: "none" | "loading" | "success" | "error" = "none";
  let useAbsolutePosition = false;
  let observer: MutationObserver | null = null;

  function checkPosition() {
    const mainContainer = document.querySelector(".main-container");
    const main = document.querySelector("main");
    if (mainContainer && main) {
      const containerRect = mainContainer.getBoundingClientRect();
      const mainRect = main.getBoundingClientRect();
      const viewportHeight = window.innerHeight;

      // If main content bottom is above viewport bottom (page is shorter than viewport)
      // Check if the bottom of main-container is above the viewport bottom with some buffer
      useAbsolutePosition = containerRect.bottom + 100 < viewportHeight;
    }
  }

  onMount(() => {
    checkPosition();
    window.addEventListener("resize", checkPosition);
    window.addEventListener("scroll", checkPosition);

    // Watch for DOM changes that might affect layout
    const mainContainer = document.querySelector(".main-container");
    if (mainContainer) {
      observer = new MutationObserver(() => {
        checkPosition();
      });
      observer.observe(mainContainer, {
        childList: true,
        subtree: true,
        attributes: true,
        attributeFilter: ["class", "style"],
      });
    }
  });

  onDestroy(() => {
    window.removeEventListener("resize", checkPosition);
    window.removeEventListener("scroll", checkPosition);
    if (observer) {
      observer.disconnect();
    }
  });

  async function handleSave() {
    console.log("Saving config...");

    // Validate config before saving
    const validationResult = validateConfigForSave(config);
    if (!validationResult.valid) {
      console.error("Config validation failed:", validationResult.errors);

      const errorMessage = getValidationErrorSummary(validationResult);
      addToast({
        type: "error",
        message: `Cannot save configuration:\n${errorMessage}`,
        timeout: 8000,
      });

      savingStatus = "error";
      setTimeout(() => {
        savingStatus = "none";
      }, 2000);
      return;
    }

    const willRebootForMode = get(modeWillChange);
    const willRebootForName = get(pipoNameWillChange);
    const willReboot = willRebootForMode || willRebootForName;
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

      // If mode or name changed, show toast and reboot immediately
      if (willReboot) {
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

      // Call success callback after a brief delay
      setTimeout(() => {
        onSaveSuccess();
        savingStatus = "none";
      }, 1500);
    } catch (error) {
      console.error("Failed to save config:", error);
      savingStatus = "error";

      // Show error toast to inform user
      addToast({
        type: "error",
        message: "Failed to save configuration. Please try again.",
        timeout: 5000,
      });

      // Reset to normal state after showing error
      // Keep the button visible so user can retry
      setTimeout(() => {
        savingStatus = "none";
      }, 3000);
    }
  }
</script>

{#if show}
  <div
    class="floating-save-container"
    class:show
    class:absolute={useAbsolutePosition}
  >
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
      {:else if $modeWillChange || $pipoNameWillChange}
        Save and Reboot
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
    left: 50%;
    transform: translateX(-50%) translateY(20px);
    width: 100%;
    max-width: 600px;
    display: flex;
    justify-content: flex-end;
    padding: 0 24px;
    box-sizing: border-box;
    z-index: 1000;
    opacity: 0;
    transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
    pointer-events: none;
  }

  .floating-save-container.show {
    opacity: 1;
    transform: translateX(-50%) translateY(0);
    pointer-events: auto;
  }

  .floating-save-container.absolute {
    position: static;
    transform: translateX(0) translateY(0);
    margin-top: 24px;
    margin-bottom: 24px;
    width: 100%;
    left: auto;
    bottom: auto;
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
