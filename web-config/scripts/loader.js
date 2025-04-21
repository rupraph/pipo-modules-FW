async function loadScript(resource) {
  const response = await fetch(resource.attrs.src);
  const code = await response.text();
  const script = document.createElement("script");
  script.textContent = code;
  document.head.appendChild(script);
  console.log("✅ Script loaded:", resource.attrs.src);
}
async function loadIcon(resource) {
  const response = await fetch(resource.attrs.href);
  const blob = await response.blob();
  const reader = new FileReader();
  await new Promise((resolve) => {
    reader.onloadend = () => {
      const dataUrl = reader.result;
      const link = document.createElement("link");
      link.rel = "icon";
      link.href = dataUrl;
      document.head.appendChild(link);
      console.log("✅ Favicon injected as data URL");
      resolve();
    };
    reader.readAsDataURL(blob);
  });
}
async function loadStylesheet(resource) {
  const response = await fetch(resource.attrs.href);
  const css = await response.text();
  const style = document.createElement("style");
  style.textContent = css;
  document.head.appendChild(style);
  console.log("✅ CSS inlined:", resource.attrs.href);
}

function arrayBufferToBase64(buffer) {
  const bytes = new Uint8Array(buffer);
  let binary = "";
  for (let i = 0; i < bytes.length; i++)
    binary += String.fromCharCode(bytes[i]);
  return btoa(binary);
}

function extractFontName(href) {
  const file = href.split("/").pop() || "CustomFont";
  return file.split(".")[0].replace(/[^a-zA-Z0-9]/g, "") || "Font";
}

async function loadSequentially(resources) {
  for (let resource of resources) {
    try {
      if (resource.tag === "script") {
        await loadScript(resource);
      } else if (
        resource.tag === "link" &&
        resource.attrs.rel === "stylesheet"
      ) {
        await loadStylesheet(resource);
      } else if (resource.tag === "link" && resource.attrs.rel === "icon") {
        await loadIcon(resource);
      }
    } catch (err) {
      console.warn("⚠️ Failed to load resource:", resource.attrs.href, err);
    }
  }
}
