import { pipoInput } from "../pipoinput";
import type { PipoTypes } from "../types";
import { writable } from "svelte/store";

export const pipoType = writable<PipoTypes>("unknown");
export const isLive = writable<boolean>(false);

let timeout: NodeJS.Timeout;
pipoInput
  .on("connect", () => {
    clearTimeout(timeout);
    isLive.set(true);
  })
  .on("disconnect", () => {
    timeout = setTimeout(() => {
      isLive.set(false);
    }, 1000);
  });
