import { pipoio } from "../pipoio";
import { writable, get } from "svelte/store";
import { schema } from "../schema";
import type {
  AxisSchema,
  InputSettings,
  PipoConfig,
  PipoKeys,
  PipoTypes,
} from "../types";
export const pipoType = writable<PipoTypes>("unknown");
export const configValid = writable<boolean>(false);

class ConfigSave<T extends PipoTypes> {
  private previousConfig: PipoConfig<T> | null = null;
  constructor(config: PipoConfig<T> | null = null) {
    this.previousConfig = config;
  }

  diff(prev: PipoConfig<T>, next: PipoConfig<T>) {
    const diff: { path: string; value: unknown }[] = [];
    const Q: [string, string, unknown, unknown][] = Object.entries(next).map(
      // @ts-expect-error Object.entries
      ([key, value]) => [key, key, value, prev[key]]
    );
    while (Q.length) {
      const [key, path, value, prevValue] = Q.shift()!;
      if (typeof value === "object") {
        Object.entries(value as Object).forEach(([k, v]) => {
          // @ts-expect-error Object.entries
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
    this.validate(config);
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

  private validate(config: PipoConfig<T>) {
    const name = config.general.PipoName;
    if (name.length < schema.name.min || name.length > schema.name.max) {
      configValid.set(false);
      return;
    }
    const type = get(pipoType);
    const schemaForType = schema[type] as Record<keyof PipoKeys[T], AxisSchema>;
    const keys = Object.keys(schemaForType) as (keyof PipoKeys[T])[];
    for (const key of keys) {
      const axis = schemaForType[key];
      // @ts-expect-error ts is dumb
      const value = config.inputs[key];
      if (value < axis.min || value > axis.max) {
        configValid.set(false);
        return;
      }
    }
    configValid.set(true);
  }
}

export const configSave = new ConfigSave();
