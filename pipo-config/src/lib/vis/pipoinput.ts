import EventEmitter from "eventemitter3";
import type { PipoEvents } from "./types";
const NOTE_ON = 0x90;
const NOTE_OFF = 0x80;
export let error = "";
function parseCC(msg: string) {
  const match = msg.match(/(CC)(\d+),(\d+),(\d+),(\d+)/);
  if (!match) return;
  let [whole, , channel, control, v, hires] = match;
  return {
    channel: Number(channel),
    control: Number(control),
    msg: msg.slice(match.index, whole.length),
    value: Number(v),
    hires: Boolean(Number(hires)),
  };
}
function parseNoteonoff(msg: string) {
  const match = msg.match(/(noteon|noteoff|cc)(\d+),(\d+),(\d+)/);
  if (!match) return;
  const [whole, cmd, channel, note, velocity] = msg;
  return {
    cmd,
    msg: msg.slice(match.index, whole.length),
    channel: Number(channel),
    note: Number(note),
    velocity: Number(velocity),
  };
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
      this.initWebSocket();
    }
  }
  retryConnection() {
    if (this.timeout) window.clearTimeout(this.timeout);
    this.timeout = window.setTimeout(() => {
      try {
        this.initWebSocket();
      } catch (e) {
        error = "Cannot init webSocket";
      }
    }, 1000);
  }
  initWebSocket() {
    // const socket = new WebSocket("ws://localhost/ws");
    return;
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
      let matches = true;
      let msg = e.data;
      // while (matches && msg.length) {
      const noteonoff = parseNoteonoff(msg);
      if (noteonoff) {
        if (noteonoff.cmd === "noteon") {
          this.emit("noteOn", {
            note: noteonoff.note,
            velocity: noteonoff.velocity,
          });
        } else {
          this.emit("noteOff", {
            note: noteonoff.note,
          });
        }
        msg = noteonoff.msg;
        matches = !!noteonoff;
      }
      const cc = parseCC(msg);
      if (!cc) {
        matches = false;
        // continue;
      }
      this.emit("controlChange", {
        control: cc.control,
        value: cc.value,
      });
      msg = cc.msg;
      // }
    });
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
