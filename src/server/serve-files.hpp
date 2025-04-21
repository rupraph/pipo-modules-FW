#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

class PipoFileServer {
 public:
  void setup(AsyncWebServer& server) {
    // Set up the server to serve static files from LittleFS
    server.onNotFound([](AsyncWebServerRequest* request) {
      // pipoDebugHeap("enter debug");
      Serial.println(request->url());
      String path = "/webpage" + request->url();

      if (path.endsWith("/")) {
        path += "index.html";  // Default to index.html if URL ends with /
      }

      // Determine MIME type
      String mimeType = "text/plain";
      if (path.endsWith(".html"))
        mimeType = "text/html";
      else if (path.endsWith(".css"))
        mimeType = "text/css";
      else if (path.endsWith(".js"))
        mimeType = "application/javascript";
      else if (path.endsWith(".json"))
        mimeType = "application/json";
      else if (path.endsWith(".png"))
        mimeType = "image/png";
      else if (path.endsWith(".jpg") || path.endsWith(".jpeg"))
        mimeType = "image/jpeg";
      else if (path.endsWith(".gif"))
        mimeType = "image/gif";
      else if (path.endsWith(".svg"))
        mimeType = "image/svg+xml";
      else if (path.endsWith(".woff"))
        mimeType = "font/woff";
      else if (path.endsWith(".woff2"))
        mimeType = "font/woff2";
      else if (path.endsWith(".ttf"))
        mimeType = "font/ttf";

      String gzPath = path + ".gz";

      if (LittleFS.exists(gzPath)) {
        path = gzPath;  // Use the original path if .gz version doesn't exist
      } else if (!LittleFS.exists(path)) {
        Serial.printf("File not found: %s\n", path.c_str());
        request->send(404, "text/plain", "File not found");
        return;
      }

      Serial.printf("Serving file: %s\n", path.c_str());

      File file = LittleFS.open(path, "r");
      size_t fileSize = file.size();

      // Create a shared pointer to keep the File alive
      std::shared_ptr<File> sharedFile = std::make_shared<File>(file);

      AsyncWebServerResponse* response = request->beginChunkedResponse(
          mimeType,
          [sharedFile, fileSize](uint8_t* buffer, size_t maxLen,
                                 size_t index) -> size_t {
            if (index >= fileSize) {
              sharedFile->close();
              return 0;
            }

            size_t chunkSize = min((size_t)2048, min(maxLen, fileSize - index));
            return sharedFile->read(buffer, chunkSize);
          });
      if (path.endsWith(".gz")) {
        response->addHeader("Content-Encoding", "gzip");
      }
      request->send(response);
    });
  }
};
