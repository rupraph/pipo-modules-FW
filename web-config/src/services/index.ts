import type { PipoTypes } from "../types";
import { writable } from "svelte/store";

export const pipoType = writable<PipoTypes>("unknown");
