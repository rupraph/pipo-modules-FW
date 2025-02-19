export type BaseMidiConfig = {
  channel: number;
  tl_mode: number; // 0=cc, 1 note
  enabled: boolean;
  min_input: number;
  max_input: number;
};
export type CCConfig = {
  cc_nb: number;
  cc_max: number;
  cc_min: number;
  hires: boolean;
  // interpolation_type: number; // 0=linear, 1=step, 2=log
};
export type NoteConfig = {
  scaleType: string;
  pattern: string;
  rootNote: number;
  sustain: number;
  nbOfNotes: number;
  current_scale: number[];
};
export type MidiConfig = BaseMidiConfig & CCConfig & NoteConfig;
export const BASIC = false;
export const HISTERESIS = true;
export const CONTINUOUS = false;
export const THRESHOLD = true;
export function isContinuousMode(config: InputSettings): boolean {
  return config.mode === CONTINUOUS;
}
export function isThresholdMode(config: InputSettings): boolean {
  return config.th_mode === THRESHOLD;
}
export function isHisteresisMode(config: InputSettings): boolean {
  return config.th_mode === HISTERESIS;
}
export function isBasicThresholdMode(config: InputSettings): boolean {
  return config.th_mode === THRESHOLD;
}
export type InputSettings = {
  mode: boolean;
  th_mode: boolean;
  deadzone: number;
  lmax: number;
  lmin: number;
  offset: number;
  inverted: boolean;
  cyclic: boolean;
};

export type OscConfig = {
  enabled: boolean;
  mode_raw: boolean;
  osc_max: number;
  osc_min: number;
  osc_addr: string;
};

export type GeneralConfig = {
  Wifi_mode: string;
  OSC_ENA: boolean;
  OSC_IP: string;
  OSC_PORT: number;
  MidiEnabled: boolean;
  HidEnabled: boolean;
  HidMode: number;
  PipoName: string;
};

export type HidConfig = {
  enabled: boolean;
  addr: string;
  addr2: string;
  stroke_mode: boolean;
};

export type PipoTypes = "motion" | "range" | "analog" | "unknown";
export type PipoKeys = {
  motion: "accX" | "accY" | "accZ" | "pitch" | "roll" | "yaw"| "magX" | "magY" | "magZ";
  range: "dist";
  analog:
    | "A01"
    | "A02"
    | "A03"
    | "A04"
    | "A05"
    | "A06"
    | "T1"
    | "T2"
    | "T3"
    | "T4"
    | "T5"
    | "T6";
  unknown: "";
};

export type AnalogOutKeys = "A01" | "A02" | "A03" | "A04" | "A05" | "A06";

export type AxisSchema = {
  label: string;
  cat: string;
  unit: string;
  min: number;
  max: number;
  step: number;
  index: number;
};
export type Schema = {
  [T in PipoTypes]: {
    [Key in PipoKeys[T]]: AxisSchema;
  };
};

export type SensorSettings = {
  motion: {
    relative_mode: boolean;
  };
  range: {
    hold_mode: boolean;
  };
  analog: {
    analogout: {[Key in AnalogOutKeys]: AnalogOut};
  };
  unknown: {
  };
};

export type AnalogOut = {
  pindir: boolean;
  outmode: number;
  lmax: number;
  lmin: number;
}


export type Axis<T extends PipoTypes> = [PipoKeys[T]];
export type PipoConfig<T extends PipoTypes> = {
  engine: {
    "engine-hid": {
      [Key in PipoKeys[T]]: HidConfig;
    };
    "engine-midi": {
      [Key in PipoKeys[T]]: MidiConfig;
    };
    "engine-osc": {
      [Key in PipoKeys[T]]: OscConfig;
    };
    "engine-special"?: {
      quat?: QuatConfig;
    }

  };
  general: GeneralConfig;
  sensor: {
    [Key in PipoKeys[T]]: InputSettings;
  };
  sensorconf: SensorSettings[T];
};
export type ChannelConfig = {
  sensor: InputSettings;
  hid: HidConfig;
  midi: MidiConfig;
  osc: OscConfig;
};
export type ConfigByChannel<T extends PipoTypes> = {
  [Key in PipoKeys[T]]: ChannelConfig;
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
  version: string;
  name: string;
};

export type QuatConfig = {
  enabled: boolean;
  osc_addr: string;
}


