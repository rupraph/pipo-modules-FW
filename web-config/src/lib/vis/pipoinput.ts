import EventEmitter from "eventemitter3";
import type { PipoEvents } from "./types";
const NOTE_ON = 0x90;
const NOTE_OFF = 0x80;
export let error = "";
function parse(msg: string) {
  const [command, ...args] = msg.split(",");
  return { command, args };
}
class PipoInput extends EventEmitter<PipoEvents> {
  private socket?: WebSocket;
  private enabled: boolean = true;
  private timeout: number = 0;
  constructor() {
    super();
    this.init();
  }

  async init() {
    try {
      await this.initWebMidi();
    } catch (e) {
      error = "Cannot init webMIDI";
    }
    this.initWebSocket();
  }
  retryConnection() {
    if (this.timeout) window.clearTimeout(this.timeout);
    this.timeout = window.setTimeout(() => {
      try {
        this.initWebSocket();
        error = "";
      } catch (e) {
        this.retryConnection();
        error = "Cannot init webSocket";
      }
    }, 1000);
  }
  initWebSocket() {
    // const socket = new WebSocket("ws://localhost/ws");
    const socket = new WebSocket(
      `${import.meta.env.VITE_STATIC_IP.replace(/http/, "ws")}/ws` ||
        "ws://localhost/ws"
    );
    this.socket = socket;
    socket.addEventListener("open", (event) => {});
    socket.addEventListener("error", (e) => {
      if (!this.enabled) return;
      this.retryConnection();
    });
    socket.addEventListener("close", (e) => {
      this.socket = undefined;
      if (!this.enabled) return;
      this.retryConnection();
    });
    socket.addEventListener("message", (e) => {
      const lines = e.data.split("\n");
      lines.forEach((msg) => {
        const { command, args } = parse(msg);
        const numargs = args.map(Number);
        switch (command) {
          case "sensor":
            const [axis, value] = args;
            this.emit("sensor", { axis, value: Number(value) });
            break;
          case "noteon":
          case "noteoff":
            const [channel, note, velocity] = numargs;
            this.emit(command as "noteon" | "noteoff", {
              channel,
              note,
              velocity,
            });
          case "cc":
            const [channel, control, value, hires] = numargs;
            this.emit("cc", { channel, control, value, hires });
            break;
          case "fps":
            const [frames, dt] = numargs;
            this.emit("fps", { frames, dt });
            break;
        }
      });
    });
    this.socket = socket;
  }
  async initWebMidi() {
    const access = await navigator.permissions.query({
      name: "midi",
      sysex: true,
    });

    if (access.state === "prompt") {
      throw new Error("Please allow MIDI access in your browser settings");
    }
    if (!navigator.requestMIDIAccess) {
      throw new Error("Cant access MIDI: not a secure context");
    }
    const midi = await navigator.requestMIDIAccess();
    const input = [...midi.inputs.values()].find((input) => {
      if (input.name?.match(/PipoUSB/)) {
        return true;
      }
    });
    if (!input) {
      throw new Error("Could not find PipoUSB MIDI device");
    }

    input.addEventListener("midimessage", (e) => {
      const [cmd, note, velocity] = e.data;
      if (cmd === NOTE_ON) {
        this.emit("noteOn", { note, velocity });
      } else if (cmd === NOTE_OFF) {
        this.emit("noteOff", { note });
      } else if (cmd === 0xb0) {
        this.emit("controlChange", { control: note, value: velocity });
      }
    });
  }
}

export const pipoInput = new PipoInput();
