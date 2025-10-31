// postbuild.js
import fs from "fs";
import path from "path";

const distDir = path.resolve("dist");
const manifestPath = path.join(distDir, ".vite", "manifest.json");
const indexPath = path.join(distDir, "index.html");
const loaderPath = path.resolve("scripts", "loader.js");

// Read files
const loaderCode = fs.readFileSync(loaderPath, "utf-8");
const manifest = JSON.parse(fs.readFileSync(manifestPath, "utf-8"));

const entry = Object.values(manifest).find((m) => m.isEntry);
if (!entry) throw new Error("No entry found in manifest");

const jsFile = entry.file;
const cssFiles = entry.css || [];

const resourceList = [
  ...cssFiles.map((css) => ({
    tag: "link",
    attrs: { rel: "stylesheet", href: `/${css}` },
  })),
  {
    tag: "script",
    attrs: { src: `/${jsFile}`, defer: false },
  },
];

const customLoader = `
    <script>
${loaderCode}
loadSequentially(JSON.parse(\`${JSON.stringify(resourceList, null, 2)}\`))
    </script>`;

let indexHtml = fs.readFileSync(indexPath, "utf-8");
indexHtml = indexHtml
  .replace(/<script type="module"[^>]+><\/script>/, customLoader)
  .replaceAll(/<link rel="stylesheet"[^>]+>/g, "");

// Save updated index.html
fs.writeFileSync(indexPath, indexHtml);
console.log("✅ index.html patched: assets will load sequentially.");
