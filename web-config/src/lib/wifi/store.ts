import { writable } from "svelte/store";
import type { WifiState } from "./types";

export const wifiState = writable<WifiState>({
  ssid: "",
  signal: 0,
});
