import { WebSocketServer } from "ws";
import { Plugin, ViteDevServer } from "vite";

export default (): Plugin => {
  return {
    name: "vite-ws-mock",
    configureServer: async (server: ViteDevServer) => {
      const wss = new WebSocketServer({ server: server.httpServer });
      wss.on("connection", function connection(ws) {
        ws.on("error", console.error);
        ws.on("message", function message(data) {
          console.log("received: %s", data);
        });
      });
    },
  };
};
