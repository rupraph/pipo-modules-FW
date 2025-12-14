import EventEmitter from "eventemitter3";
import type { PipoEvents } from "./lib/vis/types";
import axios, { type AxiosRequestConfig, type AxiosResponse } from "axios";
import type { PipoConfig, PipoTypes } from "./types";
import { formatNumbers } from "./utils";
export let error = "";
function parse(msg: string) {
  const [command, ...args] = msg.split(",");
  const isSensor = command.startsWith("sensor");
  const axis = isSensor ? args[0] : "";
  return { command, args, axis, isSensor };
}
export class PipoIO<T extends PipoTypes = "unknown"> extends EventEmitter<
  PipoEvents<T>
> {
  private socket?: WebSocket;
  private paused: boolean = false;
  private busy: boolean = false;
  private _currentPromise: Promise<any> = Promise.resolve();
  private resurrectInterval = 0;
  private saveTimeout = 0;
  private isConnecting = false;
  private nMsgs = 0;
  private msgLen = 0;
  private lastMsgDate = 0;
  private reconnectAttempts = 0;
  private reconnectTimeout = 0;
  private readonly maxReconnectDelay = 5000; // Max 5 seconds
  private readonly baseReconnectDelay = 500; // Start at 500ms
  constructor() {
    super();
    this.connect();
    
    // Check for dead connections every 3 seconds
    // If no messages received in that period, close socket and emit disconnect
    // This ensures detection during server reboots while being forgiving of brief pauses
    this.resurrectInterval = setInterval(() => {
      const n = this.nMsgs;
      this.nMsgs = 0;
      this.msgLen = 0;
      
      if (this.busy || this.paused || n > 0 || this.isConnecting) {
        return;
      }
      
      console.log('No websocket messages for 3s - connection appears dead');
      this.socket?.close();
      this.onDisconnect(true);
    }, 3000) as any as number;
  }

  private stopConnectionMonitoring() {
    if (this.resurrectInterval) {
      clearInterval(this.resurrectInterval);
      this.resurrectInterval = 0;
    }
  }

  async pause() {
    this.paused = true;
    // Reset message counter to prevent resurrect interval from triggering
    // during pause period (e.g., during WiFi scan)
    this.nMsgs = 1; // Set to 1 so resurrect check sees "activity"
    this.onDisconnect(false);
    await new Promise((resolve) => setTimeout(resolve, 500));
  }
  
  async resume() {
    this.paused = false;
    // Reset message counter on resume to give connection a fresh start
    this.nMsgs = 1;
    await new Promise((resolve) => setTimeout(resolve, 500));
  }
  
  private cleanup() {
    if (this.reconnectTimeout) {
      clearTimeout(this.reconnectTimeout);
      this.reconnectTimeout = 0;
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
    
    // Don't attempt reconnection if paused, already connecting, or still connected
    if (this.paused || this.isConnecting || (this.socket?.readyState === WebSocket.OPEN)) {
      console.log("Disconnected but not reconnecting (paused or already connecting)");
      return;
    }
    
    // Clear any pending reconnection
    if (this.reconnectTimeout) {
      clearTimeout(this.reconnectTimeout);
      this.reconnectTimeout = 0;
    }
    
    // Don't reconnect in the background
    // The page will reload after 5s timeout (handled in services/index.ts)
    // This prevents stale page state after network changes
    console.log("Disconnected - waiting for page reload, no automatic reconnection");
  }
  private onOpen() {
    this.isConnecting = false;
    // Reset reconnect attempts on successful connection
    this.reconnectAttempts = 0;
    
    setTimeout(() => {
      this.emit("connect");
    }, 100);
  }
  private onError(e: Event) {
    console.error(`WebSocket error: ${e}`);
  }
  private onMessage(m: MessageEvent<string>) {
    try {
      this.nMsgs++;
      this.msgLen += m.data.length;
      
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
            value: numargs[1],
            withinWindow: Boolean(numargs[2]),
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
    // Prevent multiple simultaneous connection attempts
    if (this.isConnecting || this.socket?.readyState === WebSocket.OPEN) {
      console.log('Already connecting or connected, skipping connect()');
      return;
    }
    
    this.isConnecting = true;
    const url = import.meta.env.VITE_STATIC_IP
      ? `${import.meta.env.VITE_STATIC_IP.replace(/http/, "ws")}/ws`
      : `ws://${location.hostname}/ws`;
    const socket = new WebSocket(url);
    this.socket = socket;
    socket.addEventListener("open", () => this.onOpen());
    socket.addEventListener("error", (e) => {
      this.isConnecting = false;
      this.onError(e);
    });
    socket.addEventListener("close", () => {
      this.isConnecting = false;
      // Only trigger disconnect if this was our active socket
      if (this.socket === socket) {
        this.socket = undefined;
        this.onDisconnect();
      }
    });
    socket.addEventListener("message", (m) => this.onMessage(m));
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

  private _wrap<T>(fn: () => Promise<T>): Promise<T> {
    // Chain promise but catch errors to prevent chain poisoning
    this._currentPromise = this._currentPromise
      .then(async () => {
        this.setBusy(true);
        try {
          await new Promise((resolve) => setTimeout(resolve, 25)); // Optional delay
          return await fn();
        } finally {
          this.setBusy(false);
        }
      })
      .catch(() => {}); // Catch errors to keep chain alive

    return this._currentPromise;
  }

  get<T = any, R = AxiosResponse<T>, D = any>(
    url: string,
    config?: AxiosRequestConfig<D>
  ): Promise<R> {
    return this._wrap(() => axios.get<T, R, D>(url, config)).then(async (r) => {
      await new Promise((resolve) => setTimeout(resolve, 25)); // Optional delay
      return r;
    });
  }

  post<T = any, R = AxiosResponse<T>, D = any>(
    url: string,
    data?: D,
    config?: AxiosRequestConfig<D>
  ): Promise<R> {
    return this._wrap(() => axios.post<T, R, D>(url, data, config));
  }

  request<T = any, R = AxiosResponse<T>, D = any>(
    config: AxiosRequestConfig<D>
  ): Promise<R> {
    return this._wrap(() => axios<T, R, D>(config));
  }

  getDebug() {
    return this.get("/conf-debug").then((res) => {
      console.log(res.data);
    });
  }

  destroy() {
    this.cleanup();
  }

  on<K extends keyof PipoEvents<T>>(
    event: K,
    listener: (evt: PipoEvents<T>[K]) => void
  ): this {
    // @ts-expect-error
    return super.on(event, listener);
  }
}
export const pipoio = new PipoIO();
// @ts-expect-error global
window.pipio = pipoio;

// Handle cleanup during HMR
if (import.meta.hot) {
  // hook before the page reloads
  import.meta.hot.accept(); // Accept HMR updates for this module
  import.meta.hot.dispose(() => {
    pipoio.destroy();
  });
}

