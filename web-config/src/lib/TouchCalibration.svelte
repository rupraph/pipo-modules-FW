<script lang="ts">
  import { currentConfig, pipoType } from "../services/config";
  import { schema } from "../schema";
  import { pipoio } from "../pipoio";

  let calibrating = false;
  $: type = $pipoType;

  function calibrateAllTouch() {
    if (!$currentConfig || calibrating) return;

    calibrating = true;

    // Get all touch channel names from the config
    const touchChannels = Object.keys($currentConfig.inputs)
      .filter((axis) => {
        const axisSchema = schema[type]?.[axis];
        return axisSchema?.cat === "Touch";
      })
      .join(",");

    if (!touchChannels) {
      console.warn("No touch channels found");
      calibrating = false;
      return;
    }

    pipoio
      .post("/offsetcal-list", null, { params: { channels: touchChannels } })
      .then(({ data }) => {
        if (data.status === "measuring") {
          // Start polling for completion
          pollCompletion();
        }
      })
      .catch((error) => {
        console.error("Touch offset calibration failed:", error);
        calibrating = false;
      });
  }

  function pollCompletion() {
    const pollInterval = setInterval(() => {
      pipoio
        .get("/offsetcal-status")
        .then(({ data }) => {
          if (data.status === "complete" && data.offsets) {
            // Use the returned offset values directly
            for (const [channel, offsetValue] of Object.entries(data.offsets)) {
              if ($currentConfig?.inputs[channel]) {
                $currentConfig.inputs[channel].offset = offsetValue as number;
              }
            }
            console.log(
              "All touch offset calibration completed:",
              data.offsets
            );
            clearInterval(pollInterval);
            calibrating = false;
            // Trigger config update
            if ($currentConfig) {
              currentConfig.set($currentConfig);
            }
          }
          // Continue polling if still measuring
        })
        .catch((error) => {
          console.error("All touch offset status check failed:", error);
          clearInterval(pollInterval);
          calibrating = false;
        });
    }, 500); // Poll every 500ms

    // Timeout after 10 seconds
    setTimeout(() => {
      clearInterval(pollInterval);
      if (calibrating) {
        console.error("All touch offset calibration timeout");
        calibrating = false;
      }
    }, 10000);
  }
</script>

<button
  on:click={calibrateAllTouch}
  disabled={calibrating || type !== "analog"}
>
  {calibrating ? "Calibrating..." : "Calibrate (zero) all touch"}
</button>
