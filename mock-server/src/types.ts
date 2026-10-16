import { Request } from "express";
import { PipoConfig, PipoKeys, PipoTypes } from "../../web-config/src/types";
export type ConfigGetParams = {
  config: PipoConfig<"motion">;
};

export type ConfigsGetParams = {
  name: string;
};
export type ActiveConfigGetParams = {
  name: string;
};
export type ConfigsDeleteGetParams = {
  name: string;
};
export type ConfigNewGetParams = {
  name: string;
};
export type ConfigRenamePostParams = {
  oldname: string;
  newname: string;
};
export type WifiModePostParams = {
  mode: "AP" | "STA" | "APSTA";
};

export type WifiConnectPostParams = {
  ssid: string;
  password: string;
};

export type WifiForgetPostParams = {
  ssid: string;
};

export type OffsetCalPostParams = {
  axis: string;
};

export type PresetGetParams = {
  name: string;
};

export type Preset = {
  name: string;
  description: string;
  filename: string;
};

export type ReqQ<T> = Request<{}, {}, {}, T>;
export type WifiState = {
  scanning: boolean;
  mode: "AP" | "STA" | "APSTA";
  rssi: -70;
  ssid: string;
  password: string;
  ip: string;
  apIP: string;
};

export type Info = {
  name: string;
  version: string;
  type: string;
  ip: string;
  mac: string;
};

export type Sensor = {
  name: string;
  value: number;
  monitoring: boolean;
};

export type Sensors<T extends PipoTypes> = { [Key in PipoKeys[T]]: Sensor };

export const MOTION_AXIS = [
  "accX",
  "accY",
  "accZ",
  "pitch",
  "roll",
  "yaw",
  "magX",
  "magY",
  "magZ",
  "gyroX",
  "gyroY",
  "gyroZ",
] as const;
export const RANGE_AXIS = ["dist"] as const;
export const ANALOG_AXIS = [
  "A01",
  "A02",
  "A03",
  "A04",
  "A05",
  "A06",
  "A07",
  "A08",
  "T1",
  "T2",
  "T3",
  "T4",
  "T5",
  "T6",
  "T7",
  "T8",
] as const;
export const UNKNOWN_AXIS = [""] as const;
