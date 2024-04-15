#ifndef SERVER_MANAGER_H
#define SERVER_MANAGER_H

#include <Arduino.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

void webserver_setup();
void setup_requests();
void notFound(AsyncWebServerRequest *request);

#endif //WEBSERVER_H