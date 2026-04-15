import { pipoio } from "../pipoio";
import { writable, get } from "svelte/store";
import { schema } from "../schema";
import { formatNumbers, debounce } from "../utils";
import type {
  AxisSchema,
  ConfigMeta,
  InputSettings,
  OutputMode,
  PipoConfig,
  PipoKeys,
  PipoTypes,
} from "../types";

// Existing stores
export const pipoType = writable<PipoTypes>("unknown");
export const configValid = writable<boolean>(false);

// Config state management stores
export const configMetas = writable<ConfigMeta[]>([]);
export const activeConfigName = writable<string>("");
export const currentConfig = writable<PipoConfig<PipoTypes> | null>(null);
//TODO: derive it from config, and have a global swith in config to switch modes.
export const currentMode = writable<OutputMode>("MIDI");

// Update currentMode when config changes
currentConfig.subscribe((config) => {
  if (config?.general) {
    const mode: OutputMode = config.general.MidiEnabled ? "MIDI" : config.general.OSC_ENA ? "OSC" : "MIDI";
    currentMode.set(mode);
  }
});

// UI state stores
export const configsLoading = writable<boolean>(false);
export const configsError = writable<string | null>(null);
export const configSaving = writable<boolean>(false);

// Change detection stores
export const originalConfig = writable<PipoConfig<PipoTypes> | null>(null);
export const hasUnsavedChanges = writable<boolean>(false);
export const modeWillChange = writable<boolean>(false);
export const pipoNameWillChange = writable<boolean>(false);

// Helper function to detect if output mode has changed
function hasOutputModeChanged(
  original: PipoConfig<PipoTypes> | null,
  current: PipoConfig<PipoTypes> | null
): boolean {
  if (!original || !current) return false;
  
  const originalMode: OutputMode = original.general.MidiEnabled ? "MIDI" : original.general.OSC_ENA ? "OSC" : "MIDI";
  const currentMode: OutputMode = current.general.MidiEnabled ? "MIDI" : current.general.OSC_ENA ? "OSC" : "MIDI";
  
  return originalMode !== currentMode;
}

// Helper function to detect if PipoName has changed
function hasPipoNameChanged(
  original: PipoConfig<PipoTypes> | null,
  current: PipoConfig<PipoTypes> | null
): boolean {
  if (!original || !current) return false;
  
  return original.general.PipoName !== current.general.PipoName;
}

// Polling interval to check for deep changes in config
let changeDetectionInterval: number | null = null;

function startChangeDetection() {
  if (changeDetectionInterval) return;
  
  changeDetectionInterval = window.setInterval(() => {
    const current = get(currentConfig);
    const original = get(originalConfig);
    
    if (!current || !original) {
      hasUnsavedChanges.set(false);
      modeWillChange.set(false);
      pipoNameWillChange.set(false);
      return;
    }
    
    // Deep comparison using JSON stringify
    const hasChanges = JSON.stringify(current) !== JSON.stringify(original);
    hasUnsavedChanges.set(hasChanges);
    
    // Check if output mode has changed
    const modeChanged = hasOutputModeChanged(original, current);
    modeWillChange.set(modeChanged);
    
    // Check if PipoName has changed
    const nameChanged = hasPipoNameChanged(original, current);
    pipoNameWillChange.set(nameChanged);
  }, 300); // Check every 300ms
}

function stopChangeDetection() {
  if (changeDetectionInterval) {
    window.clearInterval(changeDetectionInterval);
    changeDetectionInterval = null;
  }
}

// ConfigService class for centralized config management
class ConfigService {
  /**
   * Fetch list of all config names from the device
   */
  async fetchConfigNames(): Promise<string[]> {
    try {
      configsLoading.set(true);
      configsError.set(null);

      const response = await pipoio.get<ConfigMeta[] | string>("/configs");
      let names: string[];
      let metas: ConfigMeta[];

      if (typeof response.data === "string") {
        // Legacy CSV format fallback
        names = response.data.split(",").filter(Boolean);
        metas = names.map((n) => ({ name: n, active: false }));
      } else {
        // New JSON array format
        metas = response.data;
        names = metas.map((m) => m.name);
      }

      configMetas.set(metas);
      return names;
    } catch (err) {
      const errorMsg =
        err instanceof Error ? err.message : "Failed to fetch config names";
      configsError.set(errorMsg);
      console.error("Error fetching config names:", err);
      return [];
    } finally {
      configsLoading.set(false);
    }
  }

  /**
   * Fetch a specific config by name
   */
  async fetchConfig(name: string): Promise<PipoConfig<PipoTypes> | null> {
    try {
      configsLoading.set(true);
      configsError.set(null);

      const response = await pipoio.get<PipoConfig<PipoTypes>>("/configs", {
        params: { name },
      });

      // Format numbers in the config object
      const config = formatNumbers(response.data, 4);
      return config;
    } catch (err) {
      const errorMsg =
        err instanceof Error ? err.message : `Failed to load config: ${name}`;
      configsError.set(errorMsg);
      console.error("Error fetching config:", err);
      return null;
    } finally {
      configsLoading.set(false);
    }
  }

  /**
   * Fetch the active config name from the device
   */
  async fetchActiveConfigName(): Promise<string> {
    try {
      const response = await pipoio.get<string>("/config-active");
      const name = response.data;
      activeConfigName.set(name);
      return name;
    } catch (err) {
      const errorMsg =
        err instanceof Error
          ? err.message
          : "Failed to fetch active config name";
      configsError.set(errorMsg);
      console.error("Error fetching active config name:", err);
      return "";
    }
  }

  async refreshActiveConfig() {
    const name = await this.fetchActiveConfigName();
    if (!name) throw new Error("No active config name found");

    const config = await this.fetchConfig(name);
    if (config) {
      currentConfig.set(config);
      // Store a deep copy as the original for change detection
      originalConfig.set(JSON.parse(JSON.stringify(config)));
      hasUnsavedChanges.set(false);
    }
  }

  /**
   * Set the active config and load it
   */
  async setActiveConfig(name: string): Promise<void> {
    try {
      configsLoading.set(true);
      configsError.set(null);

      await pipoio.request({
        method: "post",
        url: "/active-config",
        params: { name },
      });

      activeConfigName.set(name);

      // Fetch and set the new config
      const config = await this.fetchConfig(name);
      if (config) {
        currentConfig.set(config);
        // Store a deep copy as the original for change detection
        originalConfig.set(JSON.parse(JSON.stringify(config)));
        hasUnsavedChanges.set(false);
      }
    } catch (err) {
      const errorMsg =
        err instanceof Error ? err.message : "Failed to set active config";
      configsError.set(errorMsg);
      console.error("Error setting active config:", err);
      throw err;
    } finally {
      configsLoading.set(false);
    }
  }

  /**
   * Save config to device (persist to filesystem)
   */
  async saveConfig(
    config: PipoConfig<PipoTypes>,
    name?: string
  ): Promise<void> {
    try {
      configSaving.set(true);
      configsError.set(null);

      const configName = name || get(activeConfigName);
      const blob = new Blob([JSON.stringify(config)], {
        type: "application/json",
      });
      const formData = new FormData();
      formData.append("file", blob, configName);

      await pipoio.request({
        method: "post",
        url: "/save",
        data: formData,
        headers: { "Content-Type": "multipart/form-data" },
      });

      // Reset change detection after successful save
      originalConfig.set(JSON.parse(JSON.stringify(config)));
      hasUnsavedChanges.set(false);

      console.log(`Config saved: ${configName}`);
    } catch (err) {
      const errorMsg =
        err instanceof Error ? err.message : "Failed to save config";
      configsError.set(errorMsg);
      console.error("Error saving config:", err);
      throw err;
    } finally {
      configSaving.set(false);
    }
  }

  public saveCurrentConfig = async (): Promise<void> => {
    const config = get(currentConfig);
    const name = get(activeConfigName);
    console.log("Auto-saving current config...");
    console.log("Config to save:", config);
    console.log("Config name:", name);
    if (config && name) {
      await this.saveConfig(config, name);
    } else {
      throw new Error("No current config to save");
    }
  };

  /**
   * Create a new config
   */
  async createConfig(name: string): Promise<void> {
    try {
      configsLoading.set(true);
      configsError.set(null);

      await pipoio.request({
        method: "post",
        url: "/config-new",
        params: { name },
      });

      // Refresh config list
      await this.fetchConfigNames();
    } catch (err) {
      const errorMsg =
        err instanceof Error ? err.message : "Failed to create config";
      configsError.set(errorMsg);
      console.error("Error creating config:", err);
      throw err;
    } finally {
      configsLoading.set(false);
    }
  }

  /**
   * Delete a config
   */
  async deleteConfig(name: string): Promise<void> {
    try {
      configsLoading.set(true);
      configsError.set(null);

      await pipoio.request({
        method: "post",
        url: "/config-delete",
        params: { name },
      });

      // Refresh config list and active config
      await this.fetchConfigNames();
      const activeName = await this.fetchActiveConfigName();
      const config = await this.fetchConfig(activeName);
      if (config) {
        currentConfig.set(config);
      }
    } catch (err) {
      const errorMsg =
        err instanceof Error ? err.message : "Failed to delete config";
      configsError.set(errorMsg);
      console.error("Error deleting config:", err);
      throw err;
    } finally {
      configsLoading.set(false);
    }
  }

  /**
   * Rename a config
   */
  async renameConfig(oldname: string, newname: string): Promise<void> {
    try {
      configsLoading.set(true);
      configsError.set(null);

      await pipoio.request({
        method: "post",
        url: "/config-rename",
        params: { oldname, newname },
      });

      // Refresh config list and update active name if needed
      await this.fetchConfigNames();
      const activeName = get(activeConfigName);
      if (activeName === oldname) {
        activeConfigName.set(newname);
      }
    } catch (err) {
      const errorMsg =
        err instanceof Error ? err.message : "Failed to rename config";
      configsError.set(errorMsg);
      console.error("Error renaming config:", err);
      throw err;
    } finally {
      configsLoading.set(false);
    }
  }

  /**
   * Duplicate a config on the device (backend file copy)
   */
  async duplicateConfig(source: string, target: string): Promise<void> {
    try {
      configsLoading.set(true);
      configsError.set(null);

      await pipoio.request({
        method: "post",
        url: "/config-duplicate",
        params: { source, target },
      });

      // Refresh config list
      await this.fetchConfigNames();
    } catch (err) {
      const errorMsg =
        err instanceof Error ? err.message : "Failed to duplicate config";
      configsError.set(errorMsg);
      console.error("Error duplicating config:", err);
      throw err;
    } finally {
      configsLoading.set(false);
    }
  }

  /**
   * Refresh all config data
   */
  async refresh(): Promise<void> {
    try {
      await this.fetchConfigNames();
      const activeName = await this.fetchActiveConfigName();
      const config = await this.fetchConfig(activeName);
      if (config) {
        currentConfig.set(config);
        // Store a deep copy as the original for change detection
        originalConfig.set(JSON.parse(JSON.stringify(config)));
        hasUnsavedChanges.set(false);
      }
    } catch (err) {
      const errorMsg =
        err instanceof Error ? err.message : "Failed to refresh configs";
      configsError.set(errorMsg);
      console.error("Error refreshing configs:", err);
    }
  }

  /**
   * Initialize config service - fetch initial data
   */
  async initialize(): Promise<void> {
    // Prevent multiple simultaneous initializations
    if (get(configsLoading)) {
      console.log("Config service already initializing, skipping...");
      return;
    }
    await this.refresh();
    // Start polling for changes after config is loaded
    startChangeDetection();
  }
}

export const configService = new ConfigService();

let isInitialized = false;

// Auto-fetch configs on connection (but only once per session)
pipoio.on("connect", () => {
  if (!isInitialized) {
    isInitialized = true;
    configService.initialize();
  }
});
