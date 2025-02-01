import { it, describe, beforeEach, afterEach } from "vitest";
import WebSocket from "ws";

describe("WebSockets", () => {
  let ws: WebSocket;
  beforeEach(async () => {
    ws = new WebSocket("http:/pipo-motion.local/ws", {
      perMessageDeflate: false,
    });
    let resolve: (value: unknown) => void;
    let reject: (reason?: any) => void;
    const p = new Promise((res, rej) => {
      resolve = res;
      reject = rej;
    });
    ws.addEventListener("open", () => resolve(null));
    ws.addEventListener("error", (e) => reject(e));
    await p;
  });
  afterEach(() => {
    ws.close();
  });
  it("Should handle fragmented frames", async () => {
    ws.on("message", (data) => {
      console.log(data.toString());
    });
    ws.on("error", (e) => {
      console.error(e);
    });

    // Send fragmented frames
    ws._sender.send(Buffer.from("monitor:"), {
      fin: false, // Not the final frame
      opcode: 1, // Text frame
    });

    ws._sender.send(Buffer.from("accX"), {
      fin: true, // Final frame
      opcode: 0, // Continuation frame
    });
    await new Promise((resolve) => setTimeout(resolve, 1000));
  });
});
