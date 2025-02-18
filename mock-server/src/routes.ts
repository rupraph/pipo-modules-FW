import type { Express, Request } from "express";
import { state } from "./state";
import {
  ActiveConfigGetParams,
  ConfigCopyPostParams,
  ConfigGetParams,
  ConfigNewGetParams,
  ConfigRenamePostParams,
  ConfigsDeleteGetParams,
  ConfigsGetParams,
  OffsetCalPostParams,
  ReqQ,
  WifiConnectPostParams,
  WifiModePostParams,
} from "./types";

export const setupRoutes = (app: Express) => {
  console.log("Setting up routes");
  app.get("/info", (req, res) => {
    console.log("GET INFO!");
    res.json(state.getInfo());
  });
  // routes for the captive portal
  // app.get("/generate_204", (req, res) => {});
  // app.get("/hotspot-detect.html", (req, res) => {});
  // app.get("/success.html", (req, res) => {});
  // app.get("/success.txt", (req, res) => {});

  app.post("/config", (req: ReqQ<ConfigGetParams>, res) => {
    const { config: newConfig } = req.query;
    if (!newConfig) {
      res.status(400).send("No config received");
      return;
    }
    try {
      // save the config
      state.setConfig(newConfig);
      res.status(200).send("Config saved");
    } catch (error) {
      res.status(500).send(`Error while saving config ${error}`);
    }
  });
  app.get("/configs/:name?", (req: ReqQ<ConfigsGetParams>, res) => {
    const { name } = req.query;
    if (!name) {
      res.status(200).send(Object.keys(state.getConfigs()).join(","));
      return;
    }
    if (!state.configs[name]) {
      res.status(500).send("Error loading config: Config not found");
      return;
    }
    res.status(200).send(state.configs[name]);
  });
  app.get("/config-active", (req, res) => {
    res.status(200).send(state.activeConfig);
  });
  app.post("/active-config", (req: ReqQ<ActiveConfigGetParams>, res) => {
    const { name } = req.query;
    if (!name) {
      res.status(400).send("No name received");
      return;
    }
    try {
      state.activeConfig = name;
      res.status(200).send("Active config set");
    } catch (e) {
      res.status(500).send(`Error while setting active config ${e}`);
    }
  });
  app.post("/config-delete", (req: ReqQ<ConfigsDeleteGetParams>, res) => {
    const { name } = req.query;
    if (!name) {
      res.status(400).send("No name received");
      return;
    }
    state.deleteConfig(name);
    res.send("Config deleted");
  });
  app.post("/config-new", (req: ReqQ<ConfigNewGetParams>, res) => {
    const { name } = req.query;
    if (!name) {
      res.status(400).send("No name received");
      return;
    }
    try {
      state.createConfig(name);
      res.status(200).send("Config created");
    } catch (e) {
      res.status(500).send(`Error while creating config ${e}`);
    }
  });
  app.post("/config-copy", (req: ReqQ<ConfigCopyPostParams>, res) => {
    const { name, config } = req.query;
    if (!name || !config) {
      res.status(400).send("No name or config received");
      return;
    }
    try {
      state.copyConfig(name, config);
      res.status(200).send("Config copied");
    } catch (e) {
      res.status(500).send(`Error while copying config ${e}`);
    }
  });
  app.post("/config-rename", (req: ReqQ<ConfigRenamePostParams>, res) => {
    const { oldname, newname } = req.query;
    if (!oldname || !newname) {
      res.status(400).send("No old or new name received");
      return;
    }
    try {
      state.renameConfig(oldname, newname);
      res.status(200).send("Config renamed");
    } catch (e) {
      res.status(500).send(`Error while renaming config ${e}`);
    }
  });
  app.post("/save", (req, res) => {
    // read the formData
    const data = req.body;
    console.log("SAVE", data, req.params, req.query);
    // TODO
    res.send("Config saved");
  });
  app.get("/reboot", (req, res) => {
    res.send("Rebooting");
    // TODO: reinit the server?
  });
  app.post("/wifi-mode", (req: ReqQ<WifiModePostParams>, res) => {
    const { mode: newMode } = req.query;
    if (!newMode) {
      res.status(400).send("No mode received");
      return;
    }
    if (newMode !== "AP" && newMode !== "STA" && newMode !== "APSTA") {
      res.status(400).send("Invalid mode");
      return;
    }
    res.status(200).send(`Try to switch to mode ${newMode}`);
    state.wifi.mode = newMode;
  });
  app.post("/wifi-connect", (req: ReqQ<WifiConnectPostParams>, res) => {
    const { ssid, password } = req.query;
    if (!ssid) {
      res.status(400).send("No ssid received");
      return;
    }
    res.status(200).send("Try to connect to wifi");
    if (state.wifi.mode === "AP") {
      state.wifi.mode = "APSTA";
    }
    state.wifi.ssid = ssid;
    state.wifi.password = password;
  });
  app.get("/wifi-state", (req, res) => {
    const wifistate = `${state.wifi.mode}\nCONNECTED ${state.wifi.ip} ${state.wifi.ssid}\nAP ${state.wifi.apIP}`;
    // const wifistate = `${state.wifi.mode}\nDISCONNECTED\nAP ${state.wifi.apIP}`;
    res.status(200).send(wifistate);
  });
  app.get("/wifi-networks", (req, res) => {
    const signals = [
      [`"SSID1"`, -50, 1, 1],
      [`"SSID2"`, -60, 1, 0],
      [`"SSID3"`, -70, 0, 1],
      [`"SSID3"`, -70, 0, 0],
    ]
      .map((s) => s.join(" "))
      .join("\n");
    res.status(200).send(`lastScan:\n${signals}`);
  });
  app.get("/wifi-start-scan", (_, res) => {
    if (state.wifi.scanning) {
      res.status(503).send("Scanning");
      return;
    }
    state.wifi.scanning = true;
    setTimeout(() => {
      state.wifi.scanning = false;
    }, 2000);
    res.status(200).send("Scan started");
  });
  app.get("/logs", (req, res) => {
    const sec = 1000;
    const now = Date.now();
    const logs = [
      [now - 3 * sec, 0, "A log"],
      [now - 2 * sec, 1, "A warning log"],
      [now - sec, 0, "An error log"],
    ]
      .map(([time, level, message]) => `${time}:(${level})${message}--`)
      .join("");
    res.send(logs);
  });
  app.get("/ping", (req, res) => {
    res.send("pong");
  });
  // app.get("/conf-debug", (req, res) => {});
  app.get("/battlevel", (req, res) => {
    res.send(state.battLevel.toString());
  });
  app.post("/offsetcal", (req: ReqQ<OffsetCalPostParams>, res) => {
    const { axis } = req.query;
    if (!axis) {
      res.status(400).send("No axis received");
      return;
    }
    try {
      // TODO
      res.status(200).send("Offset measured");
    } catch (e) {
      res.status(500).send(`Error while measuring offset ${e}`);
    }
  });
  app.post("/offsetAllTouch", (req, res) => {
    try {
      // TODO
      res.status(200).send("Offset measured");
    } catch (e) {
      res.status(500).send(`Error while measuring offset ${e}`);
    }
  });
  app.get("/pause", (req, res) => {
    res.send("Engine paused");
  });
};
