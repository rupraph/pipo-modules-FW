#include "server/captive-portal.h"

CaptivePortal captivePortal;
CaptivePortal::CaptivePortal() {}

void CaptivePortal::start(AsyncWebServer* server) {
  Serial.print("Starting captive portal? ");
  if (dns_server != nullptr || !canEnable()) {
    Serial.println("Nope");
    return;
  }
  return;
  Serial.println("Yes");
  dns_server = new DNSServer();
  dns_server->setErrorReplyCode(DNSReplyCode::NoError);
  // TODO: if Pipo changes IP ip, we need to update this
  dns_server->start(53, "*", WiFi.softAPIP());
  server->on("/generate_204", HTTP_GET,
             [](AsyncWebServerRequest* request) { request->redirect("/"); });

  server->on("/hotspot-detect.html", HTTP_GET,
             [](AsyncWebServerRequest* request) { request->redirect("/"); });

  server->on("/success.html", HTTP_GET,
             [](AsyncWebServerRequest* request) { request->redirect("/"); });
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

  // // Get the domain name of the incoming DNS request
  // String requestedDomain = dns_server->getDomainName();

  // if (requestedDomain.equalsIgnoreCase("pipo-motion.local")) {
  //   Serial.println("Ignoring DNS request for pipo-motion.local");
  //   return;  // Skip DNS processing, let mDNS handle this request
  // }

  // // Log other requests for debugging purposes
  // Serial.print("Processing DNS request for: ");
  // Serial.println(requestedDomain);

  // Process DNS requests normally if not pipo-motion.local
  dns_server->processNextRequest();
  // dns_server->processNextRequest();
}

bool CaptivePortal::canEnable() {
  return wifi.getStatus() == PipoWifi::CONNECTED &&
         (WiFi.getMode() == WIFI_MODE_AP || WiFi.getMode() == WIFI_MODE_APSTA);
}
bool CaptivePortal::is_active() {
  return dns_server != nullptr;
}