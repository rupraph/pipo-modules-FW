import { pipoio } from "../pipoio";
import { writable } from "svelte/store";

export * from "./config";
export * from "./presets";
export const ip = writable<string>("unknown");
export const isLive = writable<boolean>(false);
export const isLoading = writable<boolean>(true); // True on initial load

let timeout: number;
let reloadTimeout: number;

pipoio
  .on("connect", () => {
    clearTimeout(timeout);
    clearTimeout(reloadTimeout);
    isLive.set(true);
    isLoading.set(false); // First connection means we're no longer loading
  })
  .on("disconnect", () => {
    console.log("Disconnect");
    // Show overlay after 300ms - faster than first reconnect attempt (500ms)
    // This ensures users see feedback during connection issues
    // while still avoiding flashing on very quick reconnects
    timeout = setTimeout(() => {
      isLive.set(false);
    }, 300);
    
    // After 30 seconds of disconnection, reload the page
    // This handles network changes (e.g., switching from AP to STA mode)
    reloadTimeout = setTimeout(() => {
      console.log("Connection not restored after 30s - reloading page");
      window.location.reload();
    }, 30000);
  });
