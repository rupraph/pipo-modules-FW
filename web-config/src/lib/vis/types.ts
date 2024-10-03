import type { PipoKeys, PipoTypes } from "../../types";

export type PipoEvents<T extends PipoTypes> = {
  noteOn: { note: number; velocity: number };
  noteOff: { note: number };
  sensor: { axis: PipoKeys[T]; value: number; withinWindow: boolean };
  controlChange: { control: number; value: number };
  fps: { drames: number; dt: number };
  logs: { entries: string[] };
  connect: void;
  disconnect: void;
};
