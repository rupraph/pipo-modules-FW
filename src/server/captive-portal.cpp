#include "server/captive-portal.h"

CaptivePortal captivePortal;
CaptivePortal::CaptivePortal() {}

void CaptivePortal::start(AsyncWebServer* server) {
  Serial.print("Starting captive portal? ");
  if (dns_server != nullptr || !canEnable()) {
    Serial.println("Nope");
    return;
  }
  Serial.println("Yes");
  dns_server = new DNSServer();
  dns_server->setErrorReplyCode(DNSReplyCode::NoError);
  dns_server->start(53, "*", WiFi.softAPIP());
  // Handle common captive portal probe URLs
  server->on("/generate_204", HTTP_GET,
             [](AsyncWebServerRequest* request) { request->redirect("/"); });

  server->on("/hotspot-detect.html", HTTP_GET,
             [](AsyncWebServerRequest* request) { request->redirect("/"); });

  // Catch-all handler for unhandled routes (302 redirect to root)
  // server->onNotFound(
  //     [](AsyncWebServerRequest* request) { request->redirect("/"); });
}
void CaptivePortal::stop() {
  if (dns_server != nullptr) {
    dns_server->stop();
    delete dns_server;
    dns_server = nullptr;
  }
}

void CaptivePortal::loop() {
  if (dns_server == nullptr)
    return;
  dns_server->processNextRequest();
}

bool CaptivePortal::canEnable() {
  Serial.println("canEnable " + (String)(wifi.getStatus() == WL_CONNECTED) +
                 " " +
                 (String)(WiFi.getMode() == WIFI_MODE_AP ||
                          WiFi.getMode() == WIFI_MODE_APSTA));
  return wifi.getStatus() == PipoWifi::CONNECTED &&
         (WiFi.getMode() == WIFI_MODE_AP || WiFi.getMode() == WIFI_MODE_APSTA);
}
bool CaptivePortal::is_active() {
  return dns_server != nullptr;
}