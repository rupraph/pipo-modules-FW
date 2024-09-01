export function throttle(fn: Function, delay: number) {
  let last = 0;
  return function (...args: any[]) {
    const now = Date.now();
    if (now - last > delay) {
      last = now;
      fn(...args);
    }
  };
}

export function onError(e: AxiosError) {
  if (!e.config || !e.config.url) {
    return `Error during fetch: ${e}`;
  }
  const url = new URL(e.config.url);
  if (
    e.code === "ECONNABORTED" ||
    e.code === "ERR_ADDRESS_UNREACHABLE" ||
    e.code === "ERR_NETWORK"
  ) {
    return `Error fetching ${url.pathname}: Maybe Pipo is not connected to the network?
    \nPlease connect Pipo to the network and refresh the page.\n
    Tried to from ${url.origin}`;
  }
  return `Error during fetch: ${e}`;
}
export function uid() {
  return Math.random().toString(36).substring(2);
}
