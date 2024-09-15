import axios from "axios";
import { pipoInput } from "../pipoinput";
import type { PipoConfig, PipoTypes } from "../types";
import { writable } from "svelte/store";
import type { Writable } from "svelte/store";

class ConfigService<T extends PipoTypes> {
  private saveTimeout = 0;
  private store: Writable<PipoConfig<T>>;
  private previousConfig: PipoConfig<T> | null = null;
  constructor(store: Writable<PipoConfig<T>>) {
    this.store = store;
    store.subscribe((value) => {
      if (!this.previousConfig) {
        this.previousConfig = value;
        return;
      }
      const diff = clearTimeout(this.saveTimeout);
    });
  }

  diff(prev: PipoConfig<T>, next: PipoConfig<T>) {
    const diff: string[] = [];
    const path: string[] = [];
    const Q: [string, unknown][] = Object.entries(next);
    while (Q.length) {
      const [key, value] = Q.shift()!;
      if (typeof value === "object") {
        path.push(key);
        Object.entries(value as Object).forEach(([k, v]) => {
          Q.push([k, v]);
        });
      } else {
        if (prev[key] !== value) {
          diff.push(path.concat(key).join("."));
        }
      }
    }
  }

  async fetch(name?: string) {
    if (!name) {
      name = (await axios.get<string>("/config-active")).data;
    }
    const { data: config } = await axios.get<PipoConfig<T>>(`/configs`, {
      params: {
        name,
      },
    });
    this.previousConfig = null;
    this.store.set(config);
  }
}
const 