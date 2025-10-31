#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "FS.h"
#include <LittleFS.h>

class PipoFileServer : public AsyncStaticWebHandler {
  using File = fs::File;
  using FS = fs::FS;

 public:
  PipoFileServer(const char* uri, FS& fs, const char* path,
                 const char* cache_control = nullptr)
      : AsyncStaticWebHandler(uri, fs, path, cache_control) {}

  void handleRequest(AsyncWebServerRequest* request) override {
    // Get the filename from request->_tempObject and free it
    Serial.printf("FileServer running on core: %d\n", xPortGetCoreID());
    String filename = String((char*)request->_tempObject);
    free(request->_tempObject);
    request->_tempObject = NULL;

    if (request->_tempFile == true) {
      String etag = String(request->_tempFile.size());

      if (_last_modified.length() &&
          _last_modified == request->header("If-Modified-Since")) {
        request->_tempFile.close();
        request->send(304);
      } else if (_cache_control.length() &&
                 request->hasHeader("If-None-Match") &&
                 request->header("If-None-Match").equals(etag)) {
        request->_tempFile.close();
        AsyncWebServerResponse* response = new AsyncBasicResponse(304);
        response->addHeader("Cache-Control", _cache_control);
        response->addHeader("ETag", etag);
        request->send(response);
      } else {
        String mimeType = "text/plain";
        if (filename.endsWith(".html"))
          mimeType = "text/html";
        else if (filename.endsWith(".css"))
          mimeType = "text/css";
        else if (filename.endsWith(".js"))
          mimeType = "application/javascript";
        else if (filename.endsWith(".json"))
          mimeType = "application/json";
        else if (filename.endsWith(".png"))
          mimeType = "image/png";
        else if (filename.endsWith(".jpg") || filename.endsWith(".jpeg"))
          mimeType = "image/jpeg";
        else if (filename.endsWith(".gif"))
          mimeType = "image/gif";
        else if (filename.endsWith(".svg"))
          mimeType = "image/svg+xml";
        else if (filename.endsWith(".woff"))
          mimeType = "font/woff";
        else if (filename.endsWith(".woff2"))
          mimeType = "font/woff2";
        else if (filename.endsWith(".ttf"))
          mimeType = "font/ttf";

        String path = request->_tempFile.path();
        String gzPath = path + ".gz";

        if (LittleFS.exists(gzPath)) {
          path = gzPath;  // Use the original path if .gz version doesn't exist
        } else if (!LittleFS.exists(path)) {
          Serial.printf("File not found: %s\n", path.c_str());
          request->send(404, "text/plain", "File not found");
          return;
        }
        File file = request->_tempFile;
        size_t fileSize = file.size();

        // Note: Do not close the file manually; the server will handle it
        AsyncWebServerResponse* response = request->beginChunkedResponse(
            mimeType,
            [file, fileSize](uint8_t* buffer, size_t maxLen,
                             size_t index) -> size_t {
              size_t chunkSize =
                  min((size_t)2048, min(maxLen, fileSize - index));
              if (index >= fileSize || chunkSize == 0) {
                file.close();
                return 0;
              }
              int progress = (int)(100. * (float)index / (float)fileSize);
              String msg =
                  "sendChunk CH " + String(file.name()) + " " + progress + "%";
              pipoDebugHeap(msg.c_str());
              return file.read(buffer, chunkSize);
            });

        if (path.endsWith(".gz")) {
          response->addHeader("Content-Encoding", "gzip");
        }
        if (_last_modified.length())
          response->addHeader("Last-Modified", _last_modified);
        if (_cache_control.length()) {
          response->addHeader("Cache-Control", _cache_control);
          response->addHeader("ETag", etag);
        }
        request->send(response);
      }
    } else {
      request->send(404);
    }
  }
};
