import { pipoio } from "../pipoio";
import { get, writable } from "svelte/store";
import { activeConfigName, configService, currentConfig } from "./config";
import { addToast } from "../lib/toast";

export interface Preset {
  name: string;
  description: string;
  filename: string;
}

export const presets = writable<Preset[]>([]);
export const presetsLoading = writable<boolean>(false);

class PresetsService {
  async fetchPresets(): Promise<Preset[]> {
    try {
      presetsLoading.set(true);

      const response = await pipoio.get<Preset[]>("/presets");
      const presetList = response.data;

      presets.set(presetList);
      return presetList;
    } catch (err) {
      const errorMsg =
        err instanceof Error ? err.message : "Failed to fetch presets. Please reload page and try again.";
      addToast({
        type: "error",
        message: errorMsg,
        timeout: 5000,
      });
      console.error("Error fetching presets:", err);
      return [];
    } finally {
      presetsLoading.set(false);
    }
  }

  async getPreset(name: string): Promise<any> {
    try {
      const response = await pipoio.get("/preset", {
        params: { name },
      });
      return response.data;
    } catch (err) {
      const errorMsg =
        err instanceof Error ? err.message : "Failed to fetch preset list. Please reload page and try again.";
      addToast({
        type: "error",
        message: errorMsg,
        timeout: 5000,
      });
      console.error("Error loading preset:", err);
      throw err;
    }
  }

  async applyPreset(name: string): Promise<void> {
    try {
      presetsLoading.set(true);
      // Get the preset data
      const presetData = await this.getPreset(name);
      delete presetData.preset;
      
      // Preserve the current PipoName, IP, and Port (fields we don't want to overwrite)
      const current = get(currentConfig);
      if (current?.general) {
        if (current.general.PipoName) {
          presetData.general.PipoName = current.general.PipoName;
        }
        if (current.general.OSC_IP) {
          presetData.general.OSC_IP = current.general.OSC_IP;
        }
        if (current.general.OSC_PORT !== undefined) {
          presetData.general.OSC_PORT = current.general.OSC_PORT;
        }
      }
      
      // Update the current config without saving
      // This will trigger the change detection and show the floating save button
      currentConfig.set(presetData);
      
      addToast({
        type: "success",
        message: `Preset "${name}" loaded. Click Save to apply the changes.`,
        timeout: 5000,
      });
      console.log(`Loaded preset: ${name}`);
    } catch (err) {
      const errorMsg =
        err instanceof Error ? err.message : "Failed to load preset. Please reload page and try again.";
      addToast({
        type: "error",
        message: errorMsg,
        timeout: 5000,
      });
      console.error("Error loading preset:", err);
      throw err;
    } finally {
      presetsLoading.set(false);
    }
  }

  async refresh(): Promise<void> {
    try {
      await pipoio.get("/presets-refresh");
      await this.fetchPresets();
    } catch (err) {
      const errorMsg =
        err instanceof Error ? err.message : "Failed to refresh presets list. Please reload page and try again.";
      addToast({
        type: "error",
        message: errorMsg,
        timeout: 5000,
      });
      console.error("Error refreshing presets:", err);
    }
  }
}

export const presetsService = new PresetsService();

let isInitialized = false;

// Auto-fetch presets on connection (but only once per session)
pipoio.on("connect", () => {
  if (!isInitialized) {
    isInitialized = true;
    presetsService.fetchPresets();
  }
});
