export type PipoEvents = {
  noteOn: { note: number; velocity: number };
  noteOff: { note: number };
  sensor: { axis: string; value: number; withinWindow: boolean };
  controlChange: { control: number; value: number };
  fps: { drames: number; dt: number };
  logs: { entries: string[] };
  connect: void;
  disconnect: void;
};
