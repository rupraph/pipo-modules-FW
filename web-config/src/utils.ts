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

export function formatNumbers(obj: any, decimals: number): any {
  if (typeof obj === "number") {
    return parseFloat(obj.toFixed(decimals));
  } else if (Array.isArray(obj)) {
    return obj.map((item) => formatNumbers(item, decimals));
  } else if (typeof obj === "object" && obj !== null) {
    const formattedObj: any = {};
    for (const key in obj) {
      if (obj.hasOwnProperty(key)) {
        formattedObj[key] = formatNumbers(obj[key], decimals);
      }
    }
    return formattedObj;
  }
  return obj;
}
