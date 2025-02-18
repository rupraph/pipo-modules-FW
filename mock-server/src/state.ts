import defaultAnalog from "../../configs/analog_config.json";
import defaultMotion from "../../configs/motion_config.json";
import defaultRange from "../../configs/range_config.json";
import { PipoConfig, PipoKeys, PipoTypes } from "../../web-config/src/types";
import {
  ANALOG_AXIS,
  Info,
  MOTION_AXIS,
  RANGE_AXIS,
  Sensor,
  Sensors,
  UNKNOWN_AXIS,
  WifiState,
} from "./types";

class State<T extends PipoTypes = "motion"> {
  public sensors: Sensors<T>;
  public wifi: WifiState;
  public battLevel: number;
  public configs: Record<string, PipoConfig<T>>;
  private info: Info;
  public activeConfig: string;
  private type: T;
  constructor(type: T) {
    this.battLevel = 100;
    this.type = type;
    this.wifi = {
      scanning: false,
      rssi: -70,
      mode: "APSTA",
      ssid: "ESP32",
      password: "123",
      ip: "192.168.1.1",
      apIP: "192.127.127.126",
    };
    this.info = {
      name: "unnamed Pipo",
      version: "1.0.0",
      type: this.type,
      ip: global.ip,
      mac: global.mac,
    };
    this.configs = {
      default: defaultMotion as PipoConfig<"motion">,
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
  deleteConfig(name: string) {
    delete this.configs[name];
    if (!Object.keys(this.configs).length) {
      this.configs["default"] = JSON.parse(JSON.stringify(defaultMotion));
      this.activeConfig = "default";
    }
    if (this.activeConfig === name) {
      this.activeConfig = Object.keys(this.configs)[0];
    }
  }
  createConfig(name: string) {
    this.configs[name] = JSON.parse(JSON.stringify(defaultMotion));
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
}

export const state = new State("motion");
