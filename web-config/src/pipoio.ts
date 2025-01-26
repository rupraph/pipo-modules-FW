import EventEmitter from "eventemitter3";
import type { PipoEvents } from "./lib/vis/types";
import axios from "axios";
import type { PipoConfig, PipoTypes } from "./types";
import { formatNumbers } from "./utils";
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
class PipoIO<T extends PipoTypes> extends EventEmitter<PipoEvents<T>> {
  private socket?: WebSocket;
  private enabled: boolean = true;
  private timeout: number = 0;
  private bailTimeout = 0;
  private connected = false;
  private beforeReconnectTimeout = 0;
  private saveTimeout = 0;
  constructor() {
    super();
    this.init();
  }

  async init() {
    if (!this.enabled) return;
    this.initWebSocket();
  }
  async pause() {
    this.enabled = false;
    await this.onDisconnect();
    await new Promise((resolve) => setTimeout(resolve, 200));
  }
  resume() {
    this.enabled = true;
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
    }, 500);
  }
  async onDisconnect() {
    if (this.socket) {
      this.socket.close();
    }
    this.emit("disconnect");
    this.connected = false;
    if (!this.enabled) return;
    await new Promise((resolve) =>
      setTimeout(resolve, this.beforeReconnectTimeout)
    );
    this.beforeReconnectTimeout = 0;
    this.retryConnection();
  }
  onConnect() {
    this.emit("connect");
    this.connected = true;
  }
  bailOnNoNews() {
    clearTimeout(this.bailTimeout);
    this.bailTimeout = window.setTimeout(() => this.onDisconnect(), 4000);
  }
  initWebSocket() {
    const url = import.meta.env.VITE_STATIC_IP
      ? `${import.meta.env.VITE_STATIC_IP.replace(/http/, "ws")}/ws`
      : `ws://${location.hostname}/ws`;
    const socket = new WebSocket(url);
    this.socket = socket;
    this.bailOnNoNews();
    socket.addEventListener("open", () => this.onConnect());
    socket.addEventListener("error", () => {
      this.onDisconnect();
    });
    socket.addEventListener("close", () => this.onDisconnect());
    socket.addEventListener("message", (e) => {
      if (!this.connected) {
        this.onConnect();
      }
      this.bailOnNoNews();
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
          const entries = args[0].split("--");
          return this.emit("logs", { entries });
        }
      });
    });
  }

  setValue(path: string, value: unknown) {
    if (!this.socket) return;

    this.socket.send(`config:${path}:${formatNumbers(value, 4)}`);
  }

  setValues(pathvalues: { path: string; value: unknown }[]) {
    if (!this.socket) return;
    last = Date.now();
    this.socket.send(
      `configs:${pathvalues
        .map(({ path, value }) => `${path}:${formatNumbers(value, 4)}`)
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
      this.socket.send("save: ");
      this.saveTimeout = 0;
    }, 1000);
  }
  getDebug() {
    return axios.get("/conf-debug").then((res) => {
      console.log(res.data);
    });
  }
}

export const pipoio = new PipoIO();
window.pipio = pipoio;
