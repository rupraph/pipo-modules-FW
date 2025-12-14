import { pipoio } from "../pipoio";
import { writable } from "svelte/store";

export * from "./config";
export * from "./presets";
export const ip = writable<string>("unknown");
export const isLive = writable<boolean>(false);

let timeout: number;
let reloadTimeout: number;

pipoio
  .on("connect", () => {
    clearTimeout(timeout);
    clearTimeout(reloadTimeout);
    isLive.set(true);
  })
  .on("disconnect", () => {
    console.log("Disconnect");
    // Show overlay after 500ms to provide quick feedback
    // Fast enough for reboots, still filters very quick reconnects
    timeout = setTimeout(() => {
      isLive.set(false);
    }, 500);
    
    // After 30 seconds of disconnection, reload the page
    // This handles network changes (e.g., switching from AP to STA mode)
    reloadTimeout = setTimeout(() => {
      console.log("Connection not restored after 30s - reloading page");
      window.location.reload();
    }, 30000);
  });
