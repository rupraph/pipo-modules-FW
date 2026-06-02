import motionDefaults from "../../configs/motion_config.json";
import analogDefaults from "../../configs/analog2_config.json";
import rangeDefaults from "../../configs/range_config.json";
import rangeL5cxDefaults from "../../configs/range_l5cx_config.json";
import type { PipoTypes } from "./types";

type InputDefaults = Record<string, { deadband: number }>;

const configInputs: Record<Exclude<PipoTypes, "unknown">, InputDefaults> = {
  motion: motionDefaults.inputs as InputDefaults,
  analog: analogDefaults.inputs as InputDefaults,
  range: rangeDefaults.inputs as InputDefaults,
  range_l5cx: rangeL5cxDefaults.inputs as InputDefaults,
};

export function getDefaultDeadband(type: PipoTypes, channel: string): number {
  if (type === "unknown") return 0;
  return configInputs[type]?.[channel]?.deadband ?? 0;
}
