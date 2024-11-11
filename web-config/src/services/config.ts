import { pipoio } from "../pipoio";
import type { PipoConfig, PipoTypes } from "../types";

class ConfigSave<T extends PipoTypes> {
  private previousConfig: PipoConfig<T> | null = null;
  constructor(config: PipoConfig<T> | null = null) {
    this.previousConfig = config;
  }

  diff(prev: PipoConfig<T>, next: PipoConfig<T>) {
    const diff: { path: string; value: unknown }[] = [];
    const Q: [string, string, unknown, unknown][] = Object.entries(next).map(
      ([key, value]) => [key, key, value, prev[key]]
    );
    while (Q.length) {
      const [key, path, value, prevValue] = Q.shift()!;
      if (typeof value === "object") {
        Object.entries(value as Object).forEach(([k, v]) => {
          Q.push([k, `${path}/${k}`, v, prevValue[k]]);
        });
      } else {
        if (prevValue !== value) {
          diff.push({ path, value });
        }
      }
    }
    return diff;
  }

  async update(config: PipoConfig<T>) {
    if (!this.previousConfig) {
      this.previousConfig = config;
      return;
    }
    const diff = this.diff(this.previousConfig, config);
    if (!diff.length) return;
    this.previousConfig = config;
    pipoio.setValues(diff);
    pipoio.saveConfig(config);
  }
}

export const configSave = new ConfigSave();
