import { writable, get } from "svelte/store";
import { pipoio } from "../pipoio";
import { addToast } from "../lib/toast";
import {
  validateConfigForSave,
  getValidationErrorSummary,
} from "../utils/config-validator";
import {
  activeConfigName,
  modeWillChange,
  pipoNameWillChange,
  bleWillChange,
  originalConfig,
  hasUnsavedChanges,
} from "./config";
import type { PipoConfig, PipoTypes } from "../types";

// Shared state for save status
export type SavingStatus = "none" | "loading" | "success" | "error";
export const savingStatus = writable<SavingStatus>("none");

/**
 * Saves the configuration to the device
 * @param config The configuration object to save
 * @param onSuccess Callback to execute after successful save
 */
export async function saveConfig(
  config: PipoConfig<PipoTypes>,
  onSuccess?: () => void
): Promise<void> {
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

    savingStatus.set("error");
    setTimeout(() => {
      savingStatus.set("none");
    }, 2000);
    return;
  }

  const willRebootForMode = get(modeWillChange);
  const willRebootForName = get(pipoNameWillChange);
  const willRebootForBLE = get(bleWillChange);
  const willReboot = willRebootForMode || willRebootForName || willRebootForBLE;
  savingStatus.set("loading");

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
    savingStatus.set("success");

    // If mode or name changed, show toast and reboot immediately
    if (willReboot) {
      const reasons: string[] = [];
      if (willRebootForMode) reasons.push("MIDI/OSC mode change");
      if (willRebootForName) reasons.push("Pipo name change");
      if (willRebootForBLE) reasons.push("BLE change");
      const rebootReason = reasons.join(" and ");

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

    // Reset baseline immediately so new edits made during the success display
    // are detected by the change-detection polling right away.
    if (config) {
      originalConfig.set(JSON.parse(JSON.stringify(config)));
      hasUnsavedChanges.set(false);
    }

    // Keep the success label visible briefly, then clear the status.
    // The button stays rendered during this period via FloatingSaveButton's
    // shouldShow fallback ($savingStatus !== "none").
    setTimeout(() => {
      // Call custom success callback if provided
      if (onSuccess) {
        onSuccess();
      }

      savingStatus.set("none");
    }, 1500);
  } catch (error) {
    console.error("Failed to save config:", error);
    savingStatus.set("error");

    // Show error toast to inform user
    addToast({
      type: "error",
      message: "Failed to save configuration. Please try again.",
      timeout: 5000,
    });

    // Reset to normal state after showing error
    // Keep the button visible so user can retry
    setTimeout(() => {
      savingStatus.set("none");
    }, 3000);
  }
}
