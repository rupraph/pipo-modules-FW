#define DOCTEST_CONFIG_IMPLEMENT  // REQUIRED: Enable custom main()
#include <doctest.h>
#include "wifi/wifi_policy.h"

static const uint32_t S = 1000;

// Enabled, booted at t=1s, defaults: boot 60s, idle 120s, sta lost 15s
static WifiPolicyIn base(uint32_t now) {
  WifiPolicyIn in;
  in.enabled = true;
  in.bootTs = 1 * S;
  in.now = now;
  return in;
}

TEST_CASE("Disabled power save always keeps APSTA") {
  WifiPolicyIn in = base(1000 * S);
  in.enabled = false;
  in.staUpSinceTs = 10 * S;
  CHECK(decideWifiTarget(in) == WifiTarget::APSTA);
}

TEST_CASE("Boot window keeps the AP even with a stable STA") {
  WifiPolicyIn in = base(30 * S);
  in.staUpSinceTs = 5 * S;
  CHECK(decideWifiTarget(in) == WifiTarget::APSTA);
}

TEST_CASE("Stable STA after boot window drops the AP") {
  WifiPolicyIn in = base(62 * S);
  in.staUpSinceTs = 5 * S;
  CHECK(decideWifiTarget(in) == WifiTarget::STA_ONLY);
  in.oscEnabled = true;
  CHECK(decideWifiTarget(in) == WifiTarget::STA_ONLY);
}

TEST_CASE("STA must be stable before dropping the AP") {
  WifiPolicyIn in = base(100 * S);
  in.staUpSinceTs = 98 * S;
  CHECK(decideWifiTarget(in) == WifiTarget::APSTA);  // keeps current
  in.staUpSinceTs = 100 * S - WIFI_STA_STABLE_MS;
  CHECK(decideWifiTarget(in) == WifiTarget::STA_ONLY);
}

TEST_CASE("Page activity keeps the AP for the idle window") {
  WifiPolicyIn in = base(200 * S);
  in.staUpSinceTs = 5 * S;
  in.lastActivityTs = 150 * S;
  CHECK(decideWifiTarget(in) == WifiTarget::APSTA);
  in.now = 271 * S;  // 121s after last activity
  CHECK(decideWifiTarget(in) == WifiTarget::STA_ONLY);
}

TEST_CASE("Connected config page blocks AP drop") {
  WifiPolicyIn in = base(1000 * S);
  in.staUpSinceTs = 5 * S;
  in.wsClient = true;
  CHECK(decideWifiTarget(in) == WifiTarget::APSTA);
}

TEST_CASE("A device associated to the AP always keeps the AP") {
  WifiPolicyIn in = base(1000 * S);
  in.apStation = true;
  in.staUpSinceTs = 5 * S;  // stable STA: would otherwise drop to STA_ONLY
  in.oscEnabled = true;
  CHECK(decideWifiTarget(in) == WifiTarget::APSTA);
  in.oscEnabled = false;
  CHECK(decideWifiTarget(in) == WifiTarget::APSTA);
  in.staUpSinceTs = 0;  // MIDI, no STA: would otherwise turn the radio off
  CHECK(decideWifiTarget(in) == WifiTarget::APSTA);
  in.apStation = false;
  CHECK(decideWifiTarget(in) == WifiTarget::OFF);
}

TEST_CASE("OSC without STA keeps the AP indefinitely") {
  WifiPolicyIn in = base(10000 * S);
  in.oscEnabled = true;
  CHECK(decideWifiTarget(in) == WifiTarget::APSTA);
}

TEST_CASE("MIDI only without STA turns the radio off after idle") {
  WifiPolicyIn in = base(100 * S);
  in.oscEnabled = false;
  in.lastActivityTs = 50 * S;
  CHECK(decideWifiTarget(in) == WifiTarget::APSTA);
  in.now = 171 * S;
  CHECK(decideWifiTarget(in) == WifiTarget::OFF);
}

TEST_CASE("MIDI only with STA keeps STA after idle") {
  WifiPolicyIn in = base(500 * S);
  in.oscEnabled = false;
  in.staUpSinceTs = 5 * S;
  CHECK(decideWifiTarget(in) == WifiTarget::STA_ONLY);
}

TEST_CASE("Button request re-opens the AP from OFF") {
  WifiPolicyIn in = base(1000 * S);
  in.current = WifiTarget::OFF;
  CHECK(decideWifiTarget(in) == WifiTarget::OFF);
  in.apOpenTs = 1000 * S;
  CHECK(decideWifiTarget(in) == WifiTarget::APSTA);
  in.now = 1121 * S;
  in.current = WifiTarget::APSTA;
  CHECK(decideWifiTarget(in) == WifiTarget::OFF);
}

TEST_CASE("Button request re-opens the AP from STA only") {
  WifiPolicyIn in = base(1000 * S);
  in.current = WifiTarget::STA_ONLY;
  in.staUpSinceTs = 5 * S;
  in.apOpenTs = 1000 * S;
  CHECK(decideWifiTarget(in) == WifiTarget::APSTA);
}

TEST_CASE("STA lost in STA only waits for the grace period, then AP") {
  WifiPolicyIn in = base(1000 * S);
  in.current = WifiTarget::STA_ONLY;
  in.staLostSinceTs = 990 * S;
  CHECK(decideWifiTarget(in) == WifiTarget::STA_ONLY);
  in.now = 1006 * S;
  CHECK(decideWifiTarget(in) == WifiTarget::APSTA);
  in.oscEnabled = false;
  CHECK(decideWifiTarget(in) == WifiTarget::APSTA);
}

TEST_CASE("STA reconnecting during OFF-recovery does not stay OFF") {
  WifiPolicyIn in = base(1000 * S);
  in.current = WifiTarget::OFF;
  in.staUpSinceTs = 999 * S;  // not yet stable
  CHECK(decideWifiTarget(in) == WifiTarget::APSTA);
}

TEST_CASE("millis() wraparound is handled") {
  WifiPolicyIn in = base(10 * S);
  in.bootTs = 0xFFFFFFFFu - 5 * S;  // booted just before wrap
  in.oscEnabled = false;
  CHECK(decideWifiTarget(in) == WifiTarget::APSTA);  // 15s into boot window
}

int main(int argc, char** argv) {
  doctest::Context context;

  // BEGIN:: PLATFORMIO REQUIRED OPTIONS
  context.setOption("success", true);  // Report successful tests
  context.setOption("no-exitcode",
                    true);  // Do not return non-zero code on failed test case
  // END:: PLATFORMIO REQUIRED OPTIONS

  context.applyCommandLine(argc, argv);
  return context.run();
}
