#ifndef PIPOSTUBWIFI_H
#define PIPOSTUBWIFI_H
#include <WiFi.h>

String IPAddressStub::toString() {
  return "";
}

int WiFiStub::begin(String ssid, String password) {
  return 1;
}
void WiFiStub::softAP(String ssid, String password) {}
int WiFiStub::status() {
  return 0;
}
void WiFiStub::disconnect(bool force) {}
void WiFiStub::mode(int mode) {}
void WiFiStub::setAutoReconnect(bool b) {}
int WiFiStub::scanNetworks(bool a, bool b, bool c, uint d) {
  return 0;
}
int WiFiStub::SSID(int i) {
  return 0;
}
String WiFiStub::SSID() {
  return "";
}

int WiFiStub::RSSI(int i) {
  return i;
}
String WiFiStub::getString(String key, String defaultValue) {
  return "";
}
IPAddressStub WiFiStub::localIP() {
  return IPAddressStub();
}
#endif  // PIPOSTUBWIFI_H