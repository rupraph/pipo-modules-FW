import { Server as HttpServer } from "http";
import { WebSocketServer, WebSocket } from "ws";
import { state } from "./state";
import { schema } from "../../web-config/src/schema";

let lastSendTime = Date.now();
let iterations = 0;
let t = 0;
const DISCRET = 300;
function updateSens() {
  t++;
  if (t % DISCRET === 0) {
    t = 0;
  }
  const sin = Math.sin((t * 2 * Math.PI) / DISCRET);
  const s = schema[state.getType()];
  Object.keys(state.sensors).forEach((key) => {
    const min = s[key].min;
    const max = s[key].max;
    state.sensors[key].value =
      (Math.sin(Date.now() / 1000) * (max - min)) / 2 + (min + max) / 2;
    // state.sensors[key].value = 1.35; //min + ((max - min) * (sin + 1)) / 2;
  });
}
function getMsg(): string {
  const now = Date.now();
  const fps = `fps,${iterations},${now - lastSendTime}`;
  iterations = 1;
  lastSendTime = now;
  const { sensors } = state;
  const sens = Object.keys(sensors)
    .map((key) => {
      const sensor = sensors[key];
      if (!sensor.monitoring) {
        return;
      }
      return `sensor,${key},${sensor.value},${sensor.monitoring}`;
    })
    .filter((s) => s);

  const rssi = `rssi,${state.wifi.rssi}`;
  return [fps, ...sens, rssi].join("\n");
}
function updateConfig(path: string, value: string | number) {
  let obj = state.configs[state.activeConfig];
  const ps = path.split("/");
  const last = ps.pop()!;
  console.log(`Setting ${ps} ${last} to ${value}`);
  ps.forEach((key) => {
    obj = obj[key];
  });
  console.log(`ICI ${obj} to ${obj[last]}`);
  if (typeof obj[last] === "number") {
    obj[last] = Number(value);
  } else {
    obj[last] = value;
  }
}
export const setupWebSocket = (server: HttpServer) => {
  const wss = new WebSocketServer({ server });
  let shouldSendRSSI = false;
  wss.on("connection", (ws: WebSocket) => {
    console.log("New WebSocket connection");

    ws.on("message", (msg) => {
      const message = msg.toString();
      console.log(`Received message: ${message} type: ${typeof message}`);
      const [command, ...args] = message.split(":");
      if (!command || !args.length) {
        console.error("Invalid command:", message);
      }
      const arg = args.join(":");
      if (command === "config") {
        console.log(`Setting config: ${arg}`);
        const [path, value] = arg.split(":");
        updateConfig(path, value);
      } else if (command === "configs") {
        arg.split("\n").forEach((line) => {
          const [path, value] = line.split(":");
          console.log(path, value, line);

          updateConfig(path, value);
        });
        console.log(`Setting configs: ${arg}`);
      } else if (command === "save") {
        console.log("Saving config");
      } else if (command === "monitor") {
        Object.keys(state.sensors).forEach(
          (key) => (state.sensors[key].monitoring = false)
        );
        state.sensors[arg].monitoring = true;
        console.log(`Monitoring: ${arg}`);
      } else if (command === "scanrssi") {
        console.log("Scanning RSSI");
        state.wifi.rssi += Math.round(Math.random() * 2) - 1;
      } else if (command === "rssi") {
        console.log("Requesting RSSI");
        shouldSendRSSI = true;
      }
    });

    ws.on("close", () => {
      console.log("Client disconnected");
    });
  });
  setInterval(() => {
    updateSens();
    const toSend = getMsg();
    wss.clients.forEach((client) => {
      client.send(toSend);
    });
  }, 50);
  console.log("WebSocket server is running");
};
