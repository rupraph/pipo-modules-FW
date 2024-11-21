#define DOCTEST_CONFIG_IMPLEMENT  // REQUIRED: Enable custom main()
#include <ArduinoFake.h>
#include <doctest.h>
#include "wifi/pw-manager.hpp"

TEST_CASE("Should decode saved one wifi ssid and password") {
  PipoPWManager pwm;
  Preferences preferences = pwm.preferences;
  preferences.putString("indexes", "4,8,");
  preferences.putString("buffer", "ssidpassword");
  pwm.load();
  CHECK(pwm.passwords.size() == 1);
  CHECK(pwm.passwords["ssid"] == "password");
}

TEST_CASE("Should decode saved two wifi ssid and password") {
  PipoPWManager pwm;
  Preferences preferences = pwm.preferences;
  preferences.putString("indexes", "4,8,6,2,");
  preferences.putString("buffer", "ssidpasswordcoucoupa");
  pwm.load();
  CHECK(pwm.passwords.size() == 2);
  CHECK(pwm.passwords["ssid"] == "password");
  CHECK(pwm.passwords["coucou"] == "pa");
}

TEST_CASE("Should decode saved three wifi ssid and password") {
  PipoPWManager pwm;
  Preferences preferences = pwm.preferences;
  preferences.putString("indexes", "4,8,6,2,11,11,");
  preferences.putString("buffer", "ssidpasswordcoucoupasalut salutmy-password");
  pwm.load();
  CHECK(pwm.passwords.size() == 3);
  CHECK(pwm.passwords["ssid"] == "password");
  CHECK(pwm.passwords["coucou"] == "pa");
  CHECK(pwm.passwords["salut salut"] == "my-password");
}

TEST_CASE("Should encode one ssid and pasword") {
  PipoPWManager pwm;
  Preferences preferences = pwm.preferences;
  pwm.add("ssid", "password");
  CHECK(pwm.passwords.size() == 1);
  CHECK(pwm.passwords["ssid"] == "password");
  pwm.save();
  CHECK(preferences.getString("indexes") == "4,8,");
  CHECK(preferences.getString("buffer") == "ssidpassword");
}

TEST_CASE("Should encode two ssid and paswords") {
  PipoPWManager pwm;
  Preferences preferences = pwm.preferences;
  pwm.add("ssid", "password");
  pwm.add("coucou", "salut");
  CHECK(pwm.passwords.size() == 2);
  CHECK(pwm.passwords["ssid"] == "password");
  CHECK(pwm.passwords["coucou"] == "salut");
  pwm.save();
  CHECK(preferences.getString("indexes") == "6,5,4,8,");
  CHECK(preferences.getString("buffer") == "coucousalutssidpassword");
}
TEST_CASE("Should encode three ssid and paswords") {
  PipoPWManager pwm;
  Preferences preferences = pwm.preferences;
  pwm.add("ssid", "password");
  pwm.add("coucou", "salut");
  pwm.add("salut salut", "pwd");
  CHECK(pwm.passwords.size() == 3);
  CHECK(pwm.passwords["ssid"] == "password");
  CHECK(pwm.passwords["coucou"] == "salut");
  CHECK(pwm.passwords["salut salut"] == "pwd");
  pwm.save();
  CHECK(preferences.getString("indexes") == "6,5,11,3,4,8,");
  CHECK(preferences.getString("buffer") ==
        "coucousalutsalut salutpwdssidpassword");
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