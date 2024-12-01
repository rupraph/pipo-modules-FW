#include "server/captive-portal.h"

CaptivePortal captivePortal;
CaptivePortal::CaptivePortal() {}

void CaptivePortal::start() {
  if (dns_server == nullptr) {
    dns_server = new DNSServer();
    dns_server->setErrorReplyCode(DNSReplyCode::NoError);
    IPAddress ip = WiFi.softAPIP();
    WiFiUDP udp;
    udp.begin(53);
    dns_server->start(53, "*", ip);
  }
  active = true;
}
void CaptivePortal::stop() {
  if (dns_server != nullptr) {
    dns_server->stop();
    delete dns_server;
    dns_server = nullptr;
  }
  active = false;
}

void CaptivePortal::loop() {
  if (dns_server = nullptr)
    return;
  dns_server->processNextRequest();
}
bool CaptivePortal::is_active() {
  return active;
}