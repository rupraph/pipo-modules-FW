#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "FS.h"
#include <LittleFS.h>

class AsyncChunkedFileResponse : public AsyncFileResponse {
  using File = fs::File;
  using FS = fs::FS;

 public:
  AsyncChunkedFileResponse(FS& fs, const String& path,
                           const String& contentType, bool download,
                           AwsTemplateProcessor callback)
      : AsyncFileResponse(fs, path, contentType, download, callback) {
    _chunked = true;
  }

  AsyncChunkedFileResponse(File content, const String& path,
                           const String& contentType = String(),
                           bool download = false,
                           AwsTemplateProcessor callback = nullptr)
      : AsyncFileResponse(content, path, contentType, download, callback) {
    _chunked = true;
  }
};

class PipoFileServer : public AsyncStaticWebHandler {
  using File = fs::File;
  using FS = fs::FS;

 public:
  PipoFileServer(const char* uri, FS& fs, const char* path,
                 const char* cache_control = nullptr)
      : AsyncStaticWebHandler(uri, fs, path, cache_control) {}

  void handleRequest(AsyncWebServerRequest* request) override {
    // Get the filename from request->_tempObject and free it
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
        AsyncChunkedFileResponse* response = new AsyncChunkedFileResponse(
            request->_tempFile, filename, String(), false, _callback);
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
