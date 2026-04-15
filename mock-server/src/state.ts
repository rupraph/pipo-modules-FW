import defaultAnalog from "../../configs/analog2_config.json";
import defaultMotion from "../../configs/motion_config.json";
import defaultRange from "../../configs/range_config.json";
import { PipoConfig, PipoKeys, PipoTypes } from "../../web-config/src/types";
import {
  ANALOG_AXIS,
  Info,
  MOTION_AXIS,
  Preset,
  RANGE_AXIS,
  Sensor,
  Sensors,
  UNKNOWN_AXIS,
  WifiState,
} from "./types";
import * as fs from "fs";
import * as path from "path";

class State<T extends PipoTypes = "analog"> {
  public sensors: Sensors<T>;
  public wifi: WifiState;
  public battLevel: number;
  public configs: Record<string, PipoConfig<T>>;
  public presets: Preset[];
  private info: Info;
  public activeConfig: string;
  private type: T;
  public isPaused: boolean;
  constructor(type: T) {
    this.isPaused = false;
    this.battLevel = 34;
    this.type = type;
    this.wifi = {
      scanning: false,
      rssi: -70,
      mode: "APSTA",
      ssid: "Dlink-Home",
      password: "123",
      ip: "192.168.0.53",
      apIP: "192.127.127.126",
    };
    this.info = {
      name: "unnamed Pipo",
      version: "1.0.0",
      type: this.type,
      ip: global.ip,
      mac: global.mac,
    };
    
    const defaultConfig = 
      type === "motion" ? defaultMotion :
      type === "range" ? defaultRange :
      defaultAnalog;
    
    this.configs = {
      default: defaultConfig as any as PipoConfig<T>,
    };
    this.activeConfig = "default";
    const defaultSensor = (): Sensor => ({
      name: "",
      value: 0,
      monitoring: false,
    });
    const axis =
      type === "motion"
        ? MOTION_AXIS
        : type === "analog"
        ? ANALOG_AXIS
        : type === "range"
        ? RANGE_AXIS
        : UNKNOWN_AXIS;

    this.sensors = Object.fromEntries(
      axis.map((key) => [key, defaultSensor()])
    ) as Sensors<T>;

    // Initialize presets based on type
    this.presets = this.getPresetsForType(type);
  }

  private getPresetsForType(type: T): Preset[] {
    const presetType = type === "analog" ? "analog2" : type;
    const presetsPath = path.join(__dirname, "../../presets", presetType);
    const presets: Preset[] = [];

    try {
      if (!fs.existsSync(presetsPath)) {
        console.warn(`Presets directory not found: ${presetsPath}`);
        return [];
      }

      const files = fs.readdirSync(presetsPath);

      for (const filename of files) {
        if (!filename.endsWith(".json")) continue;

        const filePath = path.join(presetsPath, filename);
        try {
          const content = fs.readFileSync(filePath, "utf-8");
          const presetData = JSON.parse(content);

          if (presetData.preset && presetData.preset.name) {
            presets.push({
              name: presetData.preset.name,
              description: presetData.preset.description || "",
              filename: filename,
            });
          }
        } catch (err) {
          console.error(`Error reading preset file ${filename}:`, err);
        }
      }

      console.log(`Loaded ${presets.length} presets for type ${type}`);
    } catch (err) {
      console.error(`Error scanning presets directory:`, err);
    }

    return presets;
  }

  getPresetContent(filename: string): any {
    const presetType = this.type === "analog" ? "analog2" : this.type;
    const filePath = path.join(
      __dirname,
      "../../presets",
      presetType,
      filename
    );

    try {
      const content = fs.readFileSync(filePath, "utf-8");
      console.log(
        `Read preset file ${filePath}, ${JSON.stringify(
          JSON.parse(content).engine["engine-midi"]["A01"],
          null,
          2
        )}`
      );
      return JSON.parse(content);
    } catch (err) {
      console.error(`Error reading preset file ${filename}:`, err);
      throw new Error(`Preset file not found: ${filename}`);
    }
  }

  getInfo() {
    return {
      name: "unnamed Pipo",
      version: "1.0.0",
      type: this.type,
      // the ip of the server
      ip: global.ip,
      mac: global.mac,
    };
  }

  // setConfig(name: string, config: PipoConfig<T>) {
  //   this.configs[name] = config;
  // }
  setConfig(config: PipoConfig<T>) {
    this.configs[this.activeConfig] = config;
  }
  getConfigs() {
    return this.configs;
  }
  getConfigMetas() {
    return Object.entries(this.configs).map(([name, cfg]) => ({
      name,
      mode: (cfg as any).general?.OSC_ENA ? "osc" : "midi",
      active: name === this.activeConfig,
    }));
  }
  validateConfigName(name: string): boolean {
    return /^[a-zA-Z0-9_-]{1,12}$/.test(name);
  }
  deleteConfig(name: string) {
    delete this.configs[name];
    if (!Object.keys(this.configs).length) {
      const defaultConfig = 
        this.type === "motion" ? defaultMotion :
        this.type === "range" ? defaultRange :
        defaultAnalog;
      this.configs["default"] = JSON.parse(JSON.stringify(defaultConfig));
      this.activeConfig = "default";
    }
    if (this.activeConfig === name) {
      this.activeConfig = Object.keys(this.configs)[0];
    }
  }
  createConfig(name: string) {
    const defaultConfig = 
      this.type === "motion" ? defaultMotion :
      this.type === "range" ? defaultRange :
      defaultAnalog;
    this.configs[name] = JSON.parse(JSON.stringify(defaultConfig));
  }
  copyConfig(name: string, config: PipoConfig<T>) {
    this.configs[name] = config;
  }
  renameConfig(oldname: string, newname: string) {
    this.configs[newname] = this.configs[oldname];
    delete this.configs[oldname];
    if (oldname === this.activeConfig) {
      this.activeConfig = newname;
    }
  }
  getType() {
    return this.type;
  }
  refreshPresets() {
    this.presets = this.getPresetsForType(this.type);
  }
}

export const state = new State("motion");
