import { pipoio } from "../pipoio";
import { writable } from "svelte/store";

export interface Preset {
  name: string;
  description: string;
  filename: string;
}

export const presets = writable<Preset[]>([]);
export const presetsLoading = writable<boolean>(false);
export const presetsError = writable<string | null>(null);

class PresetsService {
  async fetchPresets(): Promise<Preset[]> {
    try {
      presetsLoading.set(true);
      presetsError.set(null);

      const response = await pipoio.get<Preset[]>("/presets");
      const presetList = response.data;

      presets.set(presetList);
      return presetList;
    } catch (err) {
      const errorMsg = err instanceof Error ? err.message : "Failed to fetch presets";
      presetsError.set(errorMsg);
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
      const errorMsg = err instanceof Error ? err.message : "Failed to load preset";
      presetsError.set(errorMsg);
      console.error("Error loading preset:", err);
      throw err;
    }
  }

  async applyPreset(name: string): Promise<void> {
    try {
      presetsLoading.set(true);
      presetsError.set(null);

      // Get the preset data
      const presetData = await this.getPreset(name);

      // Apply the preset configuration
      await pipoio.request({
        method: "post",
        url: "/config",
        params: { config: JSON.stringify(presetData) },
      });

      console.log(`Applied preset: ${name}`);
    } catch (err) {
      const errorMsg = err instanceof Error ? err.message : "Failed to apply preset";
      presetsError.set(errorMsg);
      console.error("Error applying preset:", err);
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
      const errorMsg = err instanceof Error ? err.message : "Failed to refresh presets";
      presetsError.set(errorMsg);
      console.error("Error refreshing presets:", err);
    }
  }
}

export const presetsService = new PresetsService();

// Auto-fetch presets on connection
pipoio.on("connect", () => {
  presetsService.fetchPresets();
});
