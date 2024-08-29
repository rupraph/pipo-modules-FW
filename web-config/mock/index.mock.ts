import { MockHandler } from "vite-plugin-mock-server";
import motionConfig from "./motion_config.json";
import rangeConfig from "./range_config.json";
import analogConfig from "./analog_config.json";
import { pipoType } from "./pipo-type";
const mocks: MockHandler[] = [
  {
    pattern: "/config",
    handle: (req, res) => {
      res.setHeader("Content-Type", "application/json");
      res.end(
        JSON.stringify(
          pipoType === "analaog"
            ? analogConfig
            : pipoType === "range"
            ? rangeConfig
            : motionConfig
        )
      );
    },
  },
  {
    pattern: "/info",
    handle: (req, res) => {
      console.log("info");
      res.setHeader("Content-Type", "application/json");
      res.end(
        JSON.stringify({
          type: `pipo_${pipoType}`,
        })
      );
    },
  },
  {
    pattern: "/logs",
    handle: (req, res) => {
      res.setHeader("Content-Type", "application/json");
      res.end("");
    },
  },
];

export default mocks;
