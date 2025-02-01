import { defineConfig } from "vite";
import mockServer from "vite-plugin-mock-server";
import { svelte } from "@sveltejs/vite-plugin-svelte";
import mocks from "./mock/index.mock";
import { pipoType } from "./mock/pipo-type";
const plugins = [
  svelte({
    onwarn: (warn, handler) => {
      if (warn.message.startsWith("A11y")) return;
      return handler(warn);
    },
  }),
];

console.log(process.env.motion);

if (pipoType) {
  plugins.push(
    mockServer({
      logLevel: "off",
      urlPrefixes: Object.values(mocks).map(({ pattern }) => pattern),
    })
  );
}
export default defineConfig(({ mode }) => ({
  plugins,
  envDir: pipoType ? "mocks" : ".",
}));
