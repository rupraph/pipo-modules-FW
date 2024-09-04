export type PipoEvents = {
  noteOn: { note: number; velocity: number };
  noteOff: { note: number };
  sensor: { axis: string; value: number };
  controlChange: { control: number; value: number };
};
