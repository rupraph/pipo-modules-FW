export type WifiState = {
  // mode: "client" | "ap";
  ssid: string;
  signal: number;
  lastScan: number;
  networks: Network[];
};

export type Network = {
  ssid: string;
  quality: number;
  known: boolean;
  connected: boolean;
};
