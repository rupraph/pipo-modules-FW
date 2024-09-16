import EventEmitter from "eventemitter3";
import type { PipoEvents } from "./lib/vis/types";
const NOTE_ON = 0x90;
const NOTE_OFF = 0x80;
export let error = "";
function parse(msg: string) {
  const [command, ...args] = msg.split(",");
  const isSensor = command.startsWith("sensor");
  const axis = isSensor ? command.replace("sensor", "") : "";
  return { command, args, axis, isSensor };
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
    const url = import.meta.env.VITE_STATIC_IP
      ? `${import.meta.env.VITE_STATIC_IP.replace(/http/, "ws")}/ws`
      : `ws://${location.hostname}/ws`;
    const socket = new WebSocket(url);
    this.socket = socket;
    socket.addEventListener("open", (event) => {
      console.log("Connected to Pipo");
    });
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
        const { command, args, isSensor, axis } = parse(msg);
        const numargs = args.map(Number);
        if (isSensor) {
          return this.emit("sensor", {
            axis,
            value: numargs[0],
            withinWindow: Boolean(numargs[1]),
          });
        }
        if (command === "noteon" || command === "noteoff") {
          const [channel, note, velocity] = numargs;

          return this.emit(command === "noteon" ? "noteOn" : "noteOff", {
            channel,
            note,
            velocity,
          });
        }
        if (command === "cc") {
          const [channel, control, value, hires] = numargs;
          return this.emit("controlChange", { channel, control, value, hires });
        }
        if (command === "fps") {
          const [frames, dt] = numargs;
          return this.emit("fps", { frames, dt });
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
