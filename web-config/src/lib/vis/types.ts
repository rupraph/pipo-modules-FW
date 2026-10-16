import type { PipoKeys, PipoTypes } from "../../types";

export type PipoEvents<T extends PipoTypes> = {
  connect: void;
  disconnect: void;
  sensor: {
    axis: PipoKeys[T];
    value: number;
    withinWindow: boolean;
    outputValue?: number;
  };
  rssi: {
    rssi: number;
  };
  fps: {
    frames: number;
    dt: number;
  };
  logs: {
    entries: string[];
  };
};
