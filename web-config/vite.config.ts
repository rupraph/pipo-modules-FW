import { defineConfig } from "vite";
import mockServer from "vite-plugin-mock-server";
import viteCompression from "vite-plugin-compression";
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
  viteCompression({
    deleteOriginFile: true,
    filter: /\.(js|mjs|ts|css|html|svg|json|ttf)$/,
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
