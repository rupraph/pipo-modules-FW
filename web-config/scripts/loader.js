// loader.js
function loadSequentially(resources) {
  let promise = Promise.resolve();

  for (const resource of resources) {
    promise = promise.then(() => {
      return new Promise((resolve) => {
        const el = document.createElement(resource.tag);
        Object.assign(el, resource.attrs);
        el.onload = resolve;
        document.head.appendChild(el);
      });
    });
  }
  return promise;
}
