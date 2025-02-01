import { writable } from "svelte/store";
import type { WifiState } from "./types";
import { pipoio } from "../../pipoio";
import { rssiToSignalStrength } from "./utils";

export const wifiState = writable<WifiState>({
  ssid: "",
  signal: 0,
  lastScan: 0,
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

const refreshInterval = setInterval(() => {
  console.log("interval");
  pipoio.requestRSSI();
}, 2000);

pipoio.on("rssi", ({ rssi }) => {
  setSignal(rssiToSignalStrength(rssi));
});

// Handle cleanup during HMR
if (import.meta.hot) {
  import.meta.hot.accept();
  import.meta.hot.dispose(() => {
    clearInterval(refreshInterval);
  });
}
