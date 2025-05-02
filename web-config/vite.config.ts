import { defineConfig, Plugin } from "vite";
import { readFileSync } from "fs";
import { resolve } from "path";
import mockServer from "vite-plugin-mock-server";
import viteCompression from "vite-plugin-compression";
import { svelte } from "@sveltejs/vite-plugin-svelte";
import mocks from "./mock/index.mock";
import { pipoType } from "./mock/pipo-type";

const inlineIconPlugin = (data: Record<string, string>): Plugin => ({
  name: "inline-icon",
  transformIndexHtml(html: string) {
    const rawSvg = readFileSync(resolve("src/assets/favicon.svg"), "utf8");
    const favicon = JSON.stringify(
      "data:image/svg+xml," + encodeURIComponent(rawSvg)
    );
    debugger;
    const res = html.replace(
      /<link rel=\"icon\" \/>/,
      () => `<link rel="icon" type="image/svg/xml" href=${favicon} />`
    );
    console.log("res", res);
    return res;
  },
});

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
  inlineIconPlugin({}),
];

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
  build: {
    manifest: true,
    // inline fonts to limit the amount of concurrent requests
    assetsInlineLimit: 1000 * 1024,
    rollupOptions: {
      output: {
        manualChunks: undefined,
      },
    },
  },
  envDir: pipoType ? "mocks" : ".",
}));
