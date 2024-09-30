export type BaseMidiConfig = {
  channel: number;
  translator_mode: number; // 0=cc, 1 note
  enabled: boolean;
};
export type CCConfig = {
  cc_number: number;
  max_output: number;
  min_output: number;
  hires: boolean;
  interpolation_type: number; // 0=linear, 1=step, 2=log
};
export type NoteConfig = {
  scaleType: string;
  rootNote: number;
  sustain: number;
  numberOfNotes: number;
  current_scale: number[];
};
export type MidiConfig = BaseMidiConfig & CCConfig & NoteConfig;
export const BASIC = false;
export const HISTERESIS = true;
export const CONTINUOUS = false;
export const THRESHOLD = true;
export function isContinuousMode(config: SensorConfig): boolean {
  return config.mode === CONTINUOUS;
}
export function isThresholdMode(config: SensorConfig): boolean {
  return config.threshold_mode === THRESHOLD;
}
export function isHisteresisMode(config: SensorConfig): boolean {
  return config.threshold_mode === HISTERESIS;
}
export function isBasicThresholdMode(config: SensorConfig): boolean {
  return config.threshold_mode === THRESHOLD;
}
export type SensorConfig = {
  mode: boolean;
  threshold_mode: boolean;
  deadzone: number;
  limit_max: number;
  limit_min: number;
  offset: number;
};

export type OscConfig = {
  enabled: boolean;
  mode_raw: boolean;
  output_max: number;
  output_min: number;
};

export type GeneralConfig = {
  Wifi_mode: string;
  OSC_ENA: boolean;
  OSC_IP: string;
  OSC_PORT: number;
};

export type PipoTypes = "motion" | "range" | "analog" | "unknown";
export type PipoKeys = {
  motion: "accX" | "accY" | "accZ" | "pitch" | "roll" | "yaw";
  range: "dist";
  analog:
    | "A1"
    | "A2"
    | "A3"
    | "A4"
    | "A5"
    | "A6"
    | "T1"
    | "T2"
    | "T3"
    | "T4"
    | "T5"
    | "T6";
  unknown: "";
};

export type Schema = {
  [T in PipoTypes]: {
    [Key in PipoKeys[T]]: {
      label: string;
      unit: string;
      min: number;
      max: number;
      step: number;
    };
  };
};
export type Axis<T extends PipoTypes> = [PipoKeys[T]];
export type PipoConfig<T extends PipoTypes> = {
  engine: {
    "engine-midi": {
      [Key in PipoKeys[T]]: MidiConfig;
    };
    "engine-osc": {
      [Key in PipoKeys[T]]: OscConfig;
    };
  };
  general: GeneralConfig;
  sensor: {
    [Key in PipoKeys[T]]: SensorConfig;
  };
};

export type SmoothSensorValue = {
  old: number;
  new: number;
  dt: number;
  timestamp: number;
};
export type SmoothSensorValues<T extends PipoTypes> = {
  [Key in PipoKeys[T]]: SmoothSensorValue;
};
export type SensorValues<T extends PipoTypes> = {
  [Key in PipoKeys[T]]: number | undefined;
};

export type InputMinMax = {
  [T in PipoTypes]: {
    [Key in PipoKeys[T]]: {
      min: number;
      max: number;
    };
  };
};

export type PipoInfo = {
  mac: string;
  ip: string;
  type: string;
  name: string;
};
