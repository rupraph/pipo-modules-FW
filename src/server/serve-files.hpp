#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "FS.h"
#include <LittleFS.h>

class AsyncChunkedFileResponse : public AsyncFileResponse {
  using File = fs::File;
  using FS = fs::FS;
  const size_t MAX_CHUNK_SIZE = 2048;

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

  size_t AsyncChunkedFileResponse::_ack(AsyncWebServerRequest* request,
                                        size_t len, uint32_t time) {
    (void)time;
    if (!_sourceValid()) {
      _state = RESPONSE_FAILED;
      request->client()->close();
      return 0;
    }
    _ackedLength += len;
    size_t space = min(request->client()->space(), MAX_CHUNK_SIZE);

    size_t headLen = _head.length();
    if (_state == RESPONSE_HEADERS) {
      if (space >= headLen) {
        _state = RESPONSE_CONTENT;
        space -= headLen;
      } else {
        String out = _head.substring(0, space);
        _head = _head.substring(space);
        _writtenLength += request->client()->write(out.c_str(), out.length());
        return out.length();
      }
    }

    if (_state == RESPONSE_CONTENT) {
      size_t outLen;
      if (_chunked) {
        if (space <= 8) {
          return 0;
        }
        outLen = space;
      } else if (!_sendContentLength) {
        outLen = space;
      } else {
        outLen = ((_contentLength - _sentLength) > space)
                     ? space
                     : (_contentLength - _sentLength);
      }

      uint8_t* buf = (uint8_t*)malloc(outLen + headLen);
      if (!buf) {
        // os_printf("_ack malloc %d failed\n", outLen+headLen);
        return 0;
      }

      if (headLen) {
        memcpy(buf, _head.c_str(), _head.length());
      }
      String msg = "Sending Chunks of " + String(_content.name());
      pipoDebugHeap(msg.c_str());
      size_t readLen = 0;

      if (_chunked) {
        // HTTP 1.1 allows leading zeros in chunk length. Or spaces may be added.
        // See RFC2616 sections 2, 3.6.1.
        readLen = _fillBufferAndProcessTemplates(buf + headLen + 6, outLen - 8);
        if (readLen == RESPONSE_TRY_AGAIN) {
          free(buf);
          return 0;
        }
        outLen = sprintf((char*)buf + headLen, "%x", readLen) + headLen;
        while (outLen < headLen + 4)
          buf[outLen++] = ' ';
        buf[outLen++] = '\r';
        buf[outLen++] = '\n';
        outLen += readLen;
        buf[outLen++] = '\r';
        buf[outLen++] = '\n';
      } else {
        readLen = _fillBufferAndProcessTemplates(buf + headLen, outLen);
        if (readLen == RESPONSE_TRY_AGAIN) {
          free(buf);
          return 0;
        }
        outLen = readLen + headLen;
      }

      if (headLen) {
        _head = String();
      }

      if (outLen) {
        _writtenLength += request->client()->write((const char*)buf, outLen);
      }

      if (_chunked) {
        _sentLength += readLen;
      } else {
        _sentLength += outLen - headLen;
      }

      free(buf);

      if ((_chunked && readLen == 0) || (!_sendContentLength && outLen == 0) ||
          (!_chunked && _sentLength == _contentLength)) {
        _state = RESPONSE_WAIT_ACK;
      }
      return outLen;

    } else if (_state == RESPONSE_WAIT_ACK) {
      if (!_sendContentLength || _ackedLength >= _writtenLength) {
        _state = RESPONSE_END;
        if (!_chunked && !_sendContentLength)
          request->client()->close(true);
      }
    }
    return 0;
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
