import { pipoio } from "../pipoio";
import { writable } from "svelte/store";

export * from "./config";
export * from "./presets";
export const ip = writable<string>("unknown");
export const isLive = writable<boolean>(false);

let timeout: NodeJS.Timeout;
pipoio
  .on("connect", () => {
    clearTimeout(timeout);
    isLive.set(true);
  })
  .on("disconnect", () => {
    console.log("Disconnect");
    timeout = setTimeout(() => {
      isLive.set(false);
    }, 2000);
  });
