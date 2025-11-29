#include "server/ota_update.h"
#include "utils/logs.h"

OTAUpdate otaUpdate;

bool OTAUpdate::updateInProgress = false;
size_t OTAUpdate::updateSize = 0;
size_t OTAUpdate::updateProgress = 0;
String OTAUpdate::updateError = "";

void OTAUpdate::setup(AsyncWebServer* server) {
  // Get current firmware info
  server->on("/ota/info", HTTP_GET, [](AsyncWebServerRequest* request) {
    String info = "{";
    info += "\"currentVersion\":\"";
#ifdef PIPO_FW_VERSION
    info += stringify(PIPO_FW_VERSION);
#else
    info += "unknown";
#endif
    info += "\",";
    info += "\"chipModel\":\"";
    info += ESP.getChipModel();
    info += "\",";
    info += "\"flashSize\":";
    info += ESP.getFlashChipSize();
    info += ",";
    info += "\"freeSketchSpace\":";
    info += ESP.getFreeSketchSpace();
    info += ",";
    info += "\"sketchSize\":";
    info += ESP.getSketchSize();
    info += "}";
    request->send(200, "application/json", info);
  });

  // OTA status endpoint
  server->on("/ota/status", HTTP_GET, handleOTAStatus);

  // OTA upload endpoint
  server->on(
      "/ota/upload", HTTP_POST,
      // Request completion handler
      [](AsyncWebServerRequest* request) {
        if (updateError.length() > 0) {
          request->send(
              500, "application/json",
              "{\"success\":false,\"error\":\"" + updateError + "\"}");
        } else {
          request->send(200, "application/json",
                        "{\"success\":true,\"message\":\"Update successful, "
                        "rebooting...\"}");
          delay(500);
          ESP.restart();
        }
      },
      // Upload handler
      handleOTAUpload);
}

void OTAUpdate::handleOTAUpload(AsyncWebServerRequest* request, String filename,
                                size_t index, uint8_t* data, size_t len,
                                bool final) {
  // First chunk - initialize update
  if (index == 0) {
    Serial.println("OTA Update Started: " + filename);
    updateInProgress = true;
    updateProgress = 0;
    updateError = "";

    // Calculate available space for update
    size_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;

    if (!Update.begin(maxSketchSpace, U_FLASH)) {
      updateError = "Not enough space for update";
      Update.printError(Serial);
      updateInProgress = false;
      return;
    }

    updateSize = request->contentLength();
    Serial.printf("Update size: %d bytes\n", updateSize);
  }

  // Write chunk
  if (len) {
    if (Update.write(data, len) != len) {
      updateError = "Write failed";
      Update.printError(Serial);
      updateInProgress = false;
      return;
    }
    updateProgress += len;

    // Print progress every 10%
    int progress = (updateProgress * 100) / updateSize;
    static int lastProgress = 0;
    if (progress >= lastProgress + 10) {
      Serial.printf("OTA Progress: %d%%\n", progress);
      lastProgress = progress;
    }
  }

  // Final chunk - complete update
  if (final) {
    if (Update.end(true)) {
      Serial.printf("OTA Update Success: %u bytes\n", updateProgress);
      updateInProgress = false;
    } else {
      updateError = "Update failed";
      Update.printError(Serial);
      updateInProgress = false;
    }
  }
}

void OTAUpdate::handleOTAStatus(AsyncWebServerRequest* request) {
  String status = "{";
  status += "\"inProgress\":";
  status += updateInProgress ? "true" : "false";
  status += ",\"progress\":";
  status += updateProgress;
  status += ",\"size\":";
  status += updateSize;
  if (updateError.length() > 0) {
    status += ",\"error\":\"";
    status += updateError;
    status += "\"";
  }
  status += "}";
  request->send(200, "application/json", status);
}
