import type { Express, Request } from "express";
import multer from "multer";
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
  PresetGetParams,
  ReqQ,
  WifiConnectPostParams,
  WifiForgetPostParams,
  WifiModePostParams,
} from "./types";

// Setup multer for handling multipart/form-data
const upload = multer({ storage: multer.memoryStorage() });

const signals = [
  [`"Dlink-Home"`, -40, 1, 1],
  [`"MyRouter-345"`, -50, 0, 1],
  [`"HomeSpot"`, -60, 0, 0],
  [`"WiFi-2.4-7662"`, -80, 0, 0],
];

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
  app.post("/save", upload.single("file"), (req, res) => {
    // read the formData - multer puts the file in req.file
    if (!req.file) {
      res.status(400).send("No file uploaded");
      return;
    }
    try {
      // The file buffer contains the JSON config
      const configData = req.file.buffer.toString("utf-8");
      const config = JSON.parse(configData);
      const filename = req.file.originalname;

      console.log("SAVE config:", filename);
      state.setConfig(config);
      res.status(200).send("Config saved");
    } catch (e) {
      console.error("Error saving config:", e);
      res.status(500).send(`Error while saving config: ${e}`);
    }
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
    const response = signals.map((s) => s.join(" ")).join("\n");
    res.status(200).send(`lastScan:\n${response}`);
  });
  app.post("/wifi-start-scan", (_, res) => {
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
  app.post("/wifi-forget", (req: ReqQ<WifiForgetPostParams>, res) => {
    const { ssid } = req.query;
    console.log("FORGET WIFI", ssid);
    if (!ssid) {
      res.status(400).send("Error: no ssid parameter");
      return;
    }
    const known = signals.find((s) => `"${s[0]}"` === ssid);
    if (known) {
      known[3] = 0;
    }
    console.log("FORGOTTEN WIFI", known);
    // In the mock server, we don't actually store networks, so just acknowledge
    res.status(200).send("Network forgotten");
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
  app.post("/pause", (req, res) => {
    state.isPaused = true;
    res.status(200).send("Engine paused");
  });

  app.post("/resume", (req, res) => {
    state.isPaused = false;
    res.status(200).send("Engine resumed");
  });

  app.get("/is-paused", (req, res) => {
    res.status(200).send(state.isPaused ? "true" : "false");
  });

  // Motion-specific routes
  app.get("/setreference", (req, res) => {
    console.log("Reference orientation reset");
    res.status(200).send("Reference orientation reset");
  });

  app.get("/relative-mode", (req, res) => {
    const config = state.getCurrentConfig();
    const relativeMode = config?.sensorconf?.relative_mode ?? false;
    res.status(200).send(relativeMode ? "true" : "false");
  });

  // Preset routes
  app.get("/presets", (_req, res) => {
    res.status(200).json(state.presets);
  });

  app.get("/preset", (req: ReqQ<PresetGetParams>, res) => {
    const { name } = req.query;
    if (!name) {
      res.status(400).send("Error: no name parameter");
      return;
    }

    // Find preset by name or filename
    const preset = state.presets.find(
      (p) => p.name === name || p.filename === name
    );

    if (!preset) {
      res.status(404).send("Preset not found");
      return;
    }

    try {
      // Load and return the actual preset file content
      const presetContent = state.getPresetContent(preset.filename);
      res.status(200).json(presetContent);
    } catch (err) {
      res.status(500).send(`Error loading preset: ${err}`);
    }
  });

  app.get("/presets-refresh", (_req, res) => {
    state.refreshPresets();
    res.status(200).send("Presets refreshed");
  });
};
