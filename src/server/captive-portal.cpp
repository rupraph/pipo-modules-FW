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
  // dns_server = new DNSServer();
  // dns_server->setErrorReplyCode(DNSReplyCode::NoError);
  // TODO: if Pipo changes IP ip, we need to update this
  // dns_server->start(53, "*", WiFi.softAPIP());
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
  return wifi.getStatus() == PipoWifi::CONNECTED &&
         (WiFi.getMode() == WIFI_MODE_AP || WiFi.getMode() == WIFI_MODE_APSTA);
}
bool CaptivePortal::is_active() {
  return dns_server != nullptr;
}