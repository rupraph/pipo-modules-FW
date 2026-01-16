#include "server/captive-portal.h"

CaptivePortal captivePortal;
CaptivePortal::CaptivePortal() {}

void CaptivePortal::start(AsyncWebServer* server) {
  log_d("Starting captive portal? ");
  if (dns_server != nullptr || !canEnable()) {
    log_d("Nope");
    return;
  }
  log_d("Yes");
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