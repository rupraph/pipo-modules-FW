import EventEmitter from "eventemitter3";
import type { PipoEvents } from "./lib/vis/types";
import axios, { type AxiosRequestConfig, type AxiosResponse } from "axios";
import type { PipoConfig, PipoTypes } from "./types";
import { formatNumbers } from "./utils";
export let error = "";
function parse(msg: string) {
  const [command, ...args] = msg.split(",");
  const isSensor = command.startsWith("sensor");
  const axis = isSensor ? command.replace("sensor", "") : "";
  return { command, args, axis, isSensor };
}
class PipoIO<T extends PipoTypes> extends EventEmitter<PipoEvents<T>> {
  private socket?: WebSocket;
  private paused: boolean = false;
  private busy: boolean = false;
  private heartbeat = 0;
  private saveTimeout = 0;
  private isConnecting = false;
  private resurect = 0;
  private lastMsgDate = Date.now();
  constructor() {
    super();
    this.connect();
    this.resurect = setInterval(() => {
      const now = Date.now();
      if (
        this.busy ||
        this.paused ||
        now - this.lastMsgDate < 2000 ||
        this.isConnecting
      ) {
        return;
      }
      this.onDisconnect();
      if (this.paused) return;
      this.connect();
    }, 5000) as any as number;
  }

  async pause() {
    this.paused = true;
    this.onDisconnect(false);
    await new Promise((resolve) => setTimeout(resolve, 500));
  }
  async resume() {
    this.paused = false;
    await new Promise((resolve) => setTimeout(resolve, 500));
  }
  private cleanup() {
    if (this.heartbeat) {
      clearInterval(this.heartbeat);
      this.heartbeat = 0;
    }
    if (this.socket) {
      this.socket.close();
      this.socket = undefined;
    }
  }
  async onDisconnect(sendEvent = true) {
    if (sendEvent && !this.paused) {
      this.emit("disconnect");
    }
    this.cleanup();
  }
  private onOpen() {
    setTimeout(() => {
      this.emit("connect");
    }, 100);
  }
  private onError(e: Event) {
    console.error(`WebSocket error: ${e}`);
  }
  private onMessage(m: MessageEvent<string>) {
    try {
      this.lastMsgDate = Date.now();
      const lines = m.data.split("\n");
      lines.forEach((msg) => {
        const { command, args, isSensor, axis } = parse(msg);
        const numargs = args.map(Number);
        if (command === "rssi") {
          return this.emit("rssi", { rssi: numargs[0] });
        }
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
    } catch (e) {
      console.error(`NON UTF-8 frame: `, e);
    }
  }

  private connect() {
    this.isConnecting = true;
    const url = import.meta.env.VITE_STATIC_IP
      ? `${import.meta.env.VITE_STATIC_IP.replace(/http/, "ws")}/ws`
      : `ws://${location.hostname}/ws`;
    const socket = new WebSocket(url);
    this.socket = socket;
    socket.addEventListener("open", () => this.onOpen());
    socket.addEventListener("error", (e) => this.onError(e));
    socket.addEventListener("close", () => this.onDisconnect());
    socket.addEventListener("message", (m) => this.onMessage(m));
    setTimeout(() => {
      this.isConnecting = false;
    }, 1000);
  }

  // asserts that this.socket is not null
  private canSendWSMessage() {
    return (
      this.socket &&
      this.socket.readyState === WebSocket.OPEN &&
      !this.paused &&
      !this.busy
    );
  }
  private setBusy(busy: boolean) {
    this.busy = busy;
    // reset lastMsgDate after a busy period
    if (!this.busy) this.lastMsgDate = Date.now();
  }
  monitorAxis(axis: string) {
    if (!this.canSendWSMessage()) return;
    this.socket!.send(`monitor:${axis}`);
  }

  setValue(path: string, value: unknown) {
    if (!this.canSendWSMessage()) return;
    this.socket!.send(`config:${path}:${formatNumbers(value, 4)}`);
  }

  setValues(pathvalues: { path: string; value: unknown }[]) {
    if (!this.canSendWSMessage()) return;
    this.socket!.send(
      `configs:${pathvalues
        .map(({ path, value }) => `${path}:${formatNumbers(value, 4)}`)
        .join("\n")}`
    );
  }

  requestRSSI() {
    if (!this.canSendWSMessage()) return;
    this.socket!.send("rssi:0");
  }
  saveConfig<T extends PipoTypes>(config: PipoConfig<T>) {
    if (this.saveTimeout) {
      clearTimeout(this.saveTimeout);
      this.saveTimeout = 0;
    }
    this.saveTimeout = window.setTimeout(async () => {
      if (!this.canSendWSMessage()) return;
      this.socket!.send("save: ");
      this.saveTimeout = 0;
    }, 1000);
  }

  private _wrap(p: Promise<any>) {
    return new Promise((resolve, reject) => {
      this.setBusy(true);
      setTimeout(resolve, 100);
    })
      .then(() => p)
      .finally(() => this.setBusy(false));
  }
  get<T = any, R = AxiosResponse<T>, D = any>(
    url: string,
    config?: AxiosRequestConfig<D>
  ): Promise<R> {
    return this._wrap(axios.get<T, R, D>(url, config));
  }

  post<T = any, R = AxiosResponse<T>, D = any>(
    url: string,
    data?: D,
    config?: AxiosRequestConfig<D>
  ): Promise<R> {
    return this._wrap(axios.post<T, R, D>(url, data, config));
  }

  request<T = any, R = AxiosResponse<T>, D = any>(
    config: AxiosRequestConfig<D>
  ): Promise<R> {
    return this._wrap(axios<T, R, D>(config));
  }

  getDebug() {
    return this.get("/conf-debug").then((res) => {
      console.log(res.data);
    });
  }

  destroy() {
    this.cleanup();
    clearInterval(this.resurect);
  }
}

export const pipoio = new PipoIO();
window.pipio = pipoio;

// Handle cleanup during HMR
if (import.meta.hot) {
  // hook before the page reloads
  import.meta.hot.accept(); // Accept HMR updates for this module
  import.meta.hot.dispose(() => {
    clearInterval(pipoio.resurect); // Clear the interval when the module is replaced
    pipoio.resurect = null;
  });
}
