#ifndef OTA_UPDATE_H
#define OTA_UPDATE_H

#include <Arduino.h>
#include <Update.h>
#include <ESPAsyncWebServer.h>

class OTAUpdate {
 public:
  void setup(AsyncWebServer* server);

 private:
  static void handleOTAUpload(AsyncWebServerRequest* request, String filename,
                              size_t index, uint8_t* data, size_t len,
                              bool final);
  static void handleOTAStatus(AsyncWebServerRequest* request);

  static bool updateInProgress;
  static size_t updateSize;
  static size_t updateProgress;
  static String updateError;
};

extern OTAUpdate otaUpdate;

#endif  // OTA_UPDATE_H
