import { writable } from "svelte/store";
import type { Network, WifiState } from "./types";
import { pipoio } from "../../pipoio";
import { rssiToSignalStrength } from "./utils";

export const wifiState = writable<WifiState>({
  ssid: "",
  signal: 0,
  lastScan: Date.now(),
  networks: [],
  mode: "APSTA",
  status: "DISCONNECTED",
  apIP: "unknown",
  staIP: "none",
});

export const setLastScan = (lastScan: number) => {
  wifiState.update((state) => ({ ...state, lastScan }));
};
export const setSignal = (signal: number) => {
  wifiState.update((state) => ({ ...state, signal }));
};
export const setSSID = (ssid: string) => {
  wifiState.update((state) => ({ ...state, ssid }));
};
export const setNetworks = (networks: Network[]) => {
  wifiState.update((state) => ({ ...state, networks }));
};
export const setMode = (mode: "AP" | "APSTA" | "STA") => {
  wifiState.update((state) => ({ ...state, mode }));
};
export const setConnecting = (connecting: boolean) => {
  wifiState.update((state) => ({ ...state, connecting }));
};
export const setApIP = (apIP: string) => {
  wifiState.update((state) => ({ ...state, apIP }));
};
export const setSTAIP = (staIP: string) => {
  wifiState.update((state) => ({ ...state, staIP }));
};
export const setStatus = (status: WifiState["status"]) => {
  wifiState.update((state) => ({ ...state, status }));
};

pipoio.on("rssi", ({ rssi }) => {
  const quality = rssiToSignalStrength(rssi);
  setSignal(quality);
  
  // Update the connected network's quality in the networks list
  wifiState.update((state) => {
    const networks = state.networks.map((network) => {
      if (network.connected) {
        return { ...network, quality };
      }
      return network;
    });
    return { ...state, networks };
  });
});
