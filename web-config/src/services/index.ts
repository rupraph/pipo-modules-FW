import { pipoio } from "../pipoio";
import { writable } from "svelte/store";

export * from "./config";
export * from "./presets";
export const ip = writable<string>("unknown");

// Start with overlay showing (false = not live = show overlay)
// This ensures the overlay displays during initial page load
export const isLive = writable<boolean>(false);

let showOverlayTimeout: number | undefined;
let reloadPageTimeout: number | undefined;
let disconnectTime: number = 0;
let isInitialConnection = true;

pipoio
  .on("connect", () => {
    console.log("📡 WebSocket connected");
    clearTimeout(showOverlayTimeout);
    clearTimeout(reloadPageTimeout);
    
    // Only allow initial connection, after that we reload on reconnect
    if (!isInitialConnection) {
      console.log("Reconnected after disconnect - reloading page for clean state");
      window.location.reload();
      return;
    }
    
    disconnectTime = 0;
    isInitialConnection = false;
    console.log("✅ Setting isLive to TRUE");
    isLive.set(true);
  })
  .on("disconnect", () => {
    console.log("❌ WebSocket disconnected event received");
    disconnectTime = Date.now();
    
    // Show overlay immediately on disconnect
    // User needs to know connection is lost right away
    console.log("🚨 Setting isLive to FALSE - overlay should show");
    isLive.set(false);
    
    // After 5 seconds of disconnection, reload the page
    // No point in trying to reconnect - just get a fresh page load
    reloadPageTimeout = setTimeout(() => {
      console.log("Connection lost for 5s - reloading page");
      window.location.reload();
    }, 5000) as any as number;
  });
