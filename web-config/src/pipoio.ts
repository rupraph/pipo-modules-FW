import EventEmitter from "eventemitter3";
import type { PipoEvents } from "./lib/vis/types";
import axios from "axios";
import type { PipoConfig, PipoTypes } from "./types";
const NOTE_ON = 0x90;
const NOTE_OFF = 0x80;
export let error = "";
let last = 0;
function parse(msg: string) {
  const [command, ...args] = msg.split(",");
  const isSensor = command.startsWith("sensor");
  const axis = isSensor ? command.replace("sensor", "") : "";
  return { command, args, axis, isSensor };
}
class PipoIO extends EventEmitter<PipoEvents> {
  private socket?: WebSocket;
  private enabled: boolean = true;
  private timeout: number = 0;
  private bailTimeout = 0;
  private saveTimeout = 0;
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
  onDisconnect() {
    if (this.socket) {
      this.socket.close();
    }
    // this.emit("disconnect");
    if (!this.enabled) return;
    this.retryConnection();
  }
  bailOnNoNews(delay = 1000) {
    clearTimeout(this.bailTimeout);
    this.bailTimeout = window.setTimeout(() => this.onDisconnect(), delay);
  }
  initWebSocket() {
    const url = import.meta.env.VITE_STATIC_IP
      ? `${import.meta.env.VITE_STATIC_IP.replace(/http/, "ws")}/ws`
      : `ws://${location.hostname}/ws`;
    const socket = new WebSocket(url);
    this.socket = socket;
    this.bailOnNoNews(2000);
    socket.addEventListener("open", (event) => {
      clearTimeout(this.bailTimeout);
      this.emit("connect");
      console.log("Connected to Pipo");
    });
    socket.addEventListener("error", () => this.onDisconnect());
    socket.addEventListener("close", () => this.onDisconnect());
    socket.addEventListener("message", (e) => {
      this.bailOnNoNews(2000);
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
        if (command === "logs") {
          console.log("logs", e.data);
          const entries = args[0].split("--");
          return this.emit("logs", { entries });
        }
      });
    });
  }

  setValue(path: string, value: unknown) {
    if (!this.socket) return;
    console.log(`config:${path}:${value}`);
    this.socket.send(`config:${path}:${value}`);
  }

  setValues(pathvalues: { path: string; value: unknown }[]) {
    if (!this.socket) return;
    console.log((Date.now() - last) / 1000);
    last = Date.now();
    this.socket.send(
      `configs:${pathvalues
        .map(({ path, value }) => `${path}:${value}`)
        .join("\n")}`
    );
  }
  saveConfig<T extends PipoTypes>(config: PipoConfig<T>) {
    if (this.saveTimeout) {
      clearTimeout(this.saveTimeout);
      this.saveTimeout = 0;
    }
    this.saveTimeout = window.setTimeout(async () => {
      if (!this.socket) return;
      // await axios({
      //   method: "post",
      //   url: "/save",
      //   data: JSON.stringify(config),
      //   headers: { "Content-Type": "multipart/form-data" },
      // });
      this.saveTimeout = 0;
    }, 1000);
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
  getDebug() {
    return axios.get("/conf-debug").then((res) => {
      console.log(res.data);
    });
  }
}

export const pipoio = new PipoIO();
window.pipio = pipoio;
