async function loadScript(resource) {
  const response = await fetch(resource.attrs.src);
  const code = await response.text();
  const script = document.createElement("script");
  script.textContent = code;
  document.head.appendChild(script);
  console.log("✅ Script loaded:", resource.attrs.src);
}
async function loadStylesheet(resource) {
  const response = await fetch(resource.attrs.href);
  const css = await response.text();
  const style = document.createElement("style");
  style.textContent = css;
  document.head.appendChild(style);
  console.log("✅ CSS inlined:", resource.attrs.href);
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
      }
    } catch (err) {
      console.warn("⚠️ Failed to load resource:", resource.attrs.href, err);
    }
  }
}
