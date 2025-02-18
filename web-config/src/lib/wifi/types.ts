export type WifiState = {
  // mode: "client" | "ap";
  ssid: string;
  signal: number;
  lastScan: number;
  networks: Network[];
  status: "DISCONNECTED" | "CONNECTING" | "CONNECTED";
  apIP: string;
  staIP: string;
  mode: "AP" | "STA" | "APSTA";
};

export type Network = {
  ssid: string;
  quality: number;
  known: boolean;
  connected: boolean;
};
