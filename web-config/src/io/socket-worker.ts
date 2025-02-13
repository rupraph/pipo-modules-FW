self.socket = null;

self.onmessage = function (event) {
  const { type, payload } = event.data;

  if (type === "connect") {
    if (self.socket) {
      self.socket.close();
    }
    self.socket = new WebSocket(payload.url);

    self.socket.onopen = () => {
      self.postMessage({ type: "connected" });
    };

    self.socket.onmessage = (message) => {
      self.postMessage({ type: "message", payload: message.data });
    };

    self.socket.onclose = () => {
      self.postMessage({ type: "disconnected" });
    };

    self.socket.onerror = (error) => {
      self.postMessage({ type: "error", payload: error });
    };
  } else if (type === "send" && self.socket) {
    self.socket.send(payload);
  } else if (type === "close" && self.socket) {
    self.socket.close();
  }
};
