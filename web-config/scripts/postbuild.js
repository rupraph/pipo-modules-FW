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
  {
    shouldRace: true,
    tag: "link",
    attrs: {
      rel: "icon",
      type: "image/svg+xml",
      href: "/favicon.ico",
    },
  },
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

// Replace Vite's injected <script type="module"> and remove <link rel="stylesheet">
indexHtml = indexHtml
  .replace(/<script type="module"[^>]+><\/script>/, customLoader)
  .replaceAll(/<link rel="stylesheet"[^>]+>/g, "")
  .replaceAll(/<link rel="icon"[^>]+>/g, "");

// Save updated index.html
fs.writeFileSync(indexPath, indexHtml);
console.log(indexHtml);
console.log("✅ index.html patched with sequential loader and favicon");
