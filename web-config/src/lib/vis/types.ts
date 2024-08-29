export type PipoEvents = {
  noteOn: { note: number; velocity: number };
  noteOff: { note: number };
  controlChange: { control: number; value: number };
};
