const base = 261.63;
const baseIndex = 60;
function getFrequency(note: number) {
  return base * Math.pow(2, (note - baseIndex) / 12);
}
export class Sound {
  private context: AudioContext;
  private oscillators: OscillatorNode[] = [];
  private gains: GainNode[] = [];
  private volume: number = 0.5;
  constructor() {
    const context = new AudioContext();
    // allow 12 notes at the same time
    for (let i = 0; i < 12; i++) {
      const gain = context.createGain();
      gain.gain.value = 0.0001;
      const oscillator = context.createOscillator();
      oscillator.type = "sine";
      oscillator.start();
      oscillator.connect(gain).connect(context.destination);
      this.oscillators.push(oscillator);
      this.gains.push(gain);
    }
    this.context = context;
  }

  noteOn(note: number) {
    const { context, oscillators, gains } = this;
    let index = oscillators.findIndex((o, i) => gains[i].gain.value < 0.0002);
    if (index === -1) {
      index = 0;
      gains[index].gain.exponentialRampToValueAtTime(
        0.0001,
        context.currentTime
      );
    }
    oscillators[index].frequency.value = getFrequency(note);
    gains[index].gain.exponentialRampToValueAtTime(
      this.volume,
      context.currentTime + 0.01
    );
  }
  noteOff(note: number) {
    const { context, gains, oscillators } = this;
    const freq = getFrequency(note);
    const decayTime = 2;
    oscillators.forEach((o, i) => {
      if (Math.abs(freq - +o.frequency) > 0.001) return;
      gains[i].gain.exponentialRampToValueAtTime(
        0.0001,
        context.currentTime + decayTime
      );
    });
  }
  stop() {
    this.context.close();
  }
  play() {
    const { context } = this;
    if (context.state === "running") return;
    if (context.state === "suspended") {
      context.resume();
    }
    this.volume = 0.5;
  }
  pause() {
    this.context.suspend();
    this.volume = 0.0001;
  }
}
export const sound = new Sound();
