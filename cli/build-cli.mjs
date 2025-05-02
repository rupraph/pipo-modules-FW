import { build } from "esbuild";

build({
  entryPoints: ["cli/index.ts"],
  outfile: "scripts/cli.js",
  platform: "node",
  bundle: true,
  minify: true,
  external: ["fs", "path"],
}).catch(() => process.exit(1));
