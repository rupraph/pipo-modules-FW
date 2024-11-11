#define DOCTEST_CONFIG_IMPLEMENT  // REQUIRED: Enable custom main()
#include <ArduinoFake.h>
#include <doctest.h>
#include "wifi/wifi.h"

TEST_CASE("Should decode saved wifi ssid and password") {
  PipoWifi wifi;
  Preferences preferences;
  preferences.putString("indexes", "4 8");
  preferences.putString("buffer", "ssidpassword");
  wifi.load();
  CHECK(wifi.passwords["ssid"] == "password");
}

int main(int argc, char** argv) {
  doctest::Context context;

  // BEGIN:: PLATFORMIO REQUIRED OPTIONS
  context.setOption("success", true);  // Report successful tests
  context.setOption("no-exitcode",
                    true);  // Do not return non-zero code on failed test case
  // END:: PLATFORMIO REQUIRED OPTIONS

  // YOUR CUSTOM DOCTEST OPTIONS

  context.applyCommandLine(argc, argv);
  return context.run();
}