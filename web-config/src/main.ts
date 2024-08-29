import axios from "axios";
import "./theme.css";
import "./responssive.scss";
import "./app.css";
import "./button.css";
import App from "./App.svelte";
if (import.meta.env.VITE_STATIC_IP) {
  console.log(
    "Will use this URL for PIPO API calls:",
    import.meta.env.VITE_STATIC_IP
  );
}

axios.interceptors.request.use(
  function (config) {
    if (!import.meta.env.VITE_STATIC_IP) return config;
    config.url = config.url?.replace(
      /^\//,
      `${import.meta.env.VITE_STATIC_IP}/`
    );
    return config;
  },
  function (error) {
    // Do something with request error
    return Promise.reject(error);
  }
);
const app = new App({
  target: document.getElementById("app")!,
});

export default app;
