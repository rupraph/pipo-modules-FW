#define DOCTEST_CONFIG_IMPLEMENT  // REQUIRED: Enable custom main()
#include <ArduinoFake.h>
#include <doctest.h>
#include "wifi/pw-manager.hpp"

TEST_CASE("Should decode saved one wifi ssid and password") {
  PipoPWManager pwm;
  Preferences preferences = pwm.preferences;
  preferences.putString("indexes", "4,8");
  preferences.putString("buffer", "ssidpassword");
  pwm.load();
  CHECK(pwm.passwords["ssid"] == "password");
}
TEST_CASE("Should decode saved two wifi ssid and password") {
  PipoPWManager pwm;
  Preferences preferences = pwm.preferences;
  preferences.putString("indexes", "4,8,6,2");
  preferences.putString("buffer", "ssidpasswordcoucoupa");
  pwm.load();
  CHECK(pwm.passwords.size() == 2);
  CHECK(pwm.passwords["ssid"] == "password");
  CHECK(pwm.passwords["coucou"] == "pa");
}

TEST_CASE("Should decode saved three wifi ssid and password") {
  PipoPWManager pwm;
  Preferences preferences = pwm.preferences;
  preferences.putString("indexes", "4,8,6,2,11,11");
  preferences.putString("buffer", "ssidpasswordcoucoupasalut salutmy-password");
  pwm.load();
  CHECK(pwm.passwords.size() == 3);
  CHECK(pwm.passwords["ssid"] == "password");
  CHECK(pwm.passwords["coucou"] == "pa");
  CHECK(pwm.passwords["salut salut"] == "my-password");
}

// TEST_CASE("Should encode known wifis ssid and password") {
//   PipoPWManager pwm;
//   Preferences preferences = pwm.preferences;
//   preferences.putString("indexes", "4,8");
//   preferences.putString("buffer", "ssidpassword");
//   pwm.load();
//   CHECK(pwm.passwords["ssid"] == "password");
// }

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