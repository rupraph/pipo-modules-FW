import { pipoio } from "../pipoio";
import { writable, get } from "svelte/store";
import { schema } from "../schema";
import { formatNumbers, debounce } from "../utils";
import type {
  AxisSchema,
  InputSettings,
  OutputMode,
  PipoConfig,
  PipoKeys,
  PipoTypes,
} from "../types";

// Existing stores
export const pipoType = writable<PipoTypes>("unknown");
export const configValid = writable<boolean>(false);

// New stores for config state management
export const configNames = writable<string[]>([]);
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

// Polling interval to check for deep changes in config
let changeDetectionInterval: number | null = null;

function startChangeDetection() {
  if (changeDetectionInterval) return;
  
  changeDetectionInterval = window.setInterval(() => {
    const current = get(currentConfig);
    const original = get(originalConfig);
    
    if (!current || !original) {
      hasUnsavedChanges.set(false);
      return;
    }
    
    // Deep comparison using JSON stringify
    const hasChanges = JSON.stringify(current) !== JSON.stringify(original);
    hasUnsavedChanges.set(hasChanges);
  }, 300); // Check every 300ms
}

function stopChangeDetection() {
  if (changeDetectionInterval) {
    window.clearInterval(changeDetectionInterval);
    changeDetectionInterval = null;
  }
}

class ConfigSave<T extends PipoTypes> {
  private previousConfig: PipoConfig<T> | null = null;
  private unsubscribe: (() => void) | null = null;
  private debouncedSave = debounce(() => {
    configService.saveCurrentConfig();
  }, 1000);

  constructor() {
    // Subscribe to currentConfig changes

    this.unsubscribe = currentConfig.subscribe((config) => {
      if (!config) return;
      console.log("Config changed, validating and diffing...");
      const typedConfig = config as unknown as PipoConfig<T>;
      this.validate(typedConfig);
      if (this.previousConfig) {
        const diff = this.diff(this.previousConfig, typedConfig);
        if (diff.length > 0) {
          pipoio.setValues(diff);
          this.debouncedSave();
        }
      }
      this.previousConfig = typedConfig;
    });
  }

  private diff(prev: PipoConfig<T>, next: PipoConfig<T>) {
    const diff: { path: string; value: unknown }[] = [];
    const Q: [string, string, unknown, unknown][] = Object.entries(next).map(
      // @ts-expect-error Object.entries
      ([key, value]) => [key, key, value, prev[key]]
    );
    while (Q.length) {
      const [key, path, value, prevValue] = Q.shift()!;
      if (typeof value === "object") {
        Object.entries(value as Object).forEach(([k, v]) => {
          // @ts-expect-error Object.entries
          Q.push([k, `${path}/${k}`, v, prevValue[k]]);
        });
      } else {
        if (prevValue !== value) {
          diff.push({ path, value });
        }
      }
    }
    return diff;
  }

  private validate(config: PipoConfig<T>) {
    const name = config.general.PipoName;
    if (name.length < schema.name.min || name.length > schema.name.max) {
      configValid.set(false);
      return;
    }
    const type = get(pipoType);
    const schemaForType = schema[type] as Record<keyof PipoKeys[T], AxisSchema>;
    const keys = Object.keys(schemaForType) as (keyof PipoKeys[T])[];
    for (const key of keys) {
      const axis = schemaForType[key];
      // @ts-expect-error ts is dumb
      const value = config.inputs[key];
      if (value < axis.min || value > axis.max) {
        configValid.set(false);
        return;
      }
    }
    configValid.set(true);
  }

  destroy() {
    if (this.unsubscribe) {
      this.unsubscribe();
    }
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

      const response = await pipoio.get<string>("/configs");
      const names = response.data.split(",");

      configNames.set(names);
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
   * Copy a config
   */
  async copyConfig(name: string, config: PipoConfig<PipoTypes>): Promise<void> {
    try {
      configsLoading.set(true);
      configsError.set(null);

      await pipoio.request({
        method: "post",
        url: "/config-copy",
        params: { name, config: JSON.stringify(config) },
      });

      // Refresh config list
      await this.fetchConfigNames();
    } catch (err) {
      const errorMsg =
        err instanceof Error ? err.message : "Failed to copy config";
      configsError.set(errorMsg);
      console.error("Error copying config:", err);
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
    await this.refresh();
    // Start polling for changes after config is loaded
    startChangeDetection();
  }
}

export const configService = new ConfigService();

// Initialize ConfigSave to auto-subscribe to currentConfig changes
export const configSave = new ConfigSave<PipoTypes>();

// Auto-fetch configs on connection
pipoio.on("connect", () => {
  configService.initialize();
});
