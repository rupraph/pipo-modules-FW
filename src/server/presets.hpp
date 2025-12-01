#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <vector>

struct PresetInfo {
  String name;
  String description;
  String filename;
};

class PipoPresets {
 private:
  std::vector<PresetInfo> presets;
  const char* presets_path = "/presets";

  void scanPresets() {
    presets.clear();

    File root = LittleFS.open(presets_path);
    if (!root) {
      Serial.println("Failed to open presets directory");
      return;
    }

    if (!root.isDirectory()) {
      Serial.println("Presets path is not a directory");
      root.close();
      return;
    }

    File file = root.openNextFile();
    while (file) {
      if (!file.isDirectory() && String(file.name()).endsWith(".json")) {
        String filename = String(file.name());

        // Read the JSON file to extract name and description
        size_t size = file.size();
        std::unique_ptr<char[]> buf(new char[size]);
        file.read((uint8_t*)buf.get(), size);

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, buf.get(), size);

        if (!error && doc.containsKey("preset")) {
          PresetInfo info;
          info.filename = filename;
          info.name = doc["preset"]["name"].as<String>();
          info.description = doc["preset"]["description"].as<String>();
          presets.push_back(info);

          Serial.printf("Found preset: %s - %s (%s)\n",
                       info.name.c_str(),
                       info.description.c_str(),
                       info.filename.c_str());
        } else {
          Serial.printf("Failed to parse preset file: %s\n", filename.c_str());
        }
      }
      file.close();
      file = root.openNextFile();
    }
    root.close();

    Serial.printf("Found %d presets\n", presets.size());
  }

 public:
  PipoPresets() {}

  void setup() {
    scanPresets();
  }

  void addRoutes(AsyncWebServer* server) {
    // GET /presets - Returns array of preset names and descriptions
    server->on("/presets", HTTP_GET, [this](AsyncWebServerRequest* request) {
      JsonDocument doc;
      JsonArray presetsArray = doc.to<JsonArray>();

      for (const auto& preset : presets) {
        JsonObject presetObj = presetsArray.add<JsonObject>();
        presetObj["name"] = preset.name;
        presetObj["description"] = preset.description;
        presetObj["filename"] = preset.filename;
      }

      String response;
      serializeJson(doc, response);
      request->send(200, "application/json", response);
    });

    // GET /preset/:name - Returns the JSON content of the requested preset
    server->on("/preset", HTTP_GET, [this](AsyncWebServerRequest* request) {
      if (!request->hasParam("name")) {
        return request->send(400, "text/plain", "Error: no name parameter");
      }

      String requestedName = request->getParam("name")->value();

      // Find the preset by name
      String filename = "";
      for (const auto& preset : presets) {
        if (preset.name == requestedName || preset.filename == requestedName) {
          filename = preset.filename;
          break;
        }
      }

      if (filename.isEmpty()) {
        return request->send(404, "text/plain", "Preset not found");
      }

      String filepath = String(presets_path) + "/" + filename;

      if (!LittleFS.exists(filepath)) {
        return request->send(404, "text/plain", "Preset file not found");
      }

      request->send(LittleFS, filepath, "application/json");
      Serial.printf("Served preset file: %s\n", filepath.c_str());
    });

    // GET /presets-refresh - Rescans the presets directory
    server->on("/presets-refresh", HTTP_GET, [this](AsyncWebServerRequest* request) {
      scanPresets();
      request->send(200, "text/plain", "Presets refreshed");
    });
  }
};
