#define DOCTEST_CONFIG_IMPLEMENT  // REQUIRED: Enable custom main()
#include <ArduinoFake.h>
#include <doctest.h>
#include "wifi/pw-manager.hpp"

TEST_CASE("Should decode saved one wifi ssid and password") {
  PipoPWManager pwm;
  Preferences preferences = pwm.preferences;
  preferences.putString("indexes", "4,8,");
  preferences.putString("buffer", "ssidpasswordA");
  pwm.load();
  CHECK(pwm.passwords.size() == 1);
  CHECK(pwm.passwords["ssid"] == "password");
  CHECK(pwm.scores["ssid"] == 65);
}

TEST_CASE("Should decode saved two wifi ssid and password") {
  PipoPWManager pwm;
  Preferences preferences = pwm.preferences;
  preferences.putString("indexes", "4,8,6,2,");
  preferences.putString("buffer", "ssidpasswordAcoucoupaB");
  pwm.load();
  CHECK(pwm.passwords.size() == 2);
  CHECK(pwm.passwords["ssid"] == "password");
  CHECK(pwm.scores["ssid"] == 65);
  CHECK(pwm.passwords["coucou"] == "pa");
  CHECK(pwm.scores["coucou"] == 66);
}

TEST_CASE("Should decode saved three wifi ssid and password") {
  PipoPWManager pwm;
  Preferences preferences = pwm.preferences;
  preferences.putString("indexes", "4,8,6,2,11,11,");
  preferences.putString("buffer",
                        "ssidpasswordAcoucoupaBsalut salutmy-passwordC");
  pwm.load();
  CHECK(pwm.passwords.size() == 3);
  CHECK(pwm.passwords["ssid"] == "password");
  CHECK(pwm.scores["ssid"] == 65);
  CHECK(pwm.passwords["coucou"] == "pa");
  CHECK(pwm.scores["coucou"] == 66);
  CHECK(pwm.passwords["salut salut"] == "my-password");
  CHECK(pwm.scores["salut salut"] == 67);
}

TEST_CASE("Should encode one ssid and pasword") {
  PipoPWManager pwm;
  Preferences preferences = pwm.preferences;
  pwm.add("ssid", "password");
  CHECK(pwm.passwords.size() == 1);
  CHECK(pwm.passwords["ssid"] == "password");
  CHECK(pwm.scores["ssid"] == 128);
  pwm.save();
  CHECK(preferences.getString("indexes") == "4,8,");
  CHECK(preferences.getString("buffer") == "ssidpassword\x80");
}

TEST_CASE("Should encode two ssid and paswords") {
  PipoPWManager pwm;
  Preferences preferences = pwm.preferences;
  pwm.add("ssid", "password");
  pwm.add("coucou", "salut");
  CHECK(pwm.passwords.size() == 2);
  CHECK(pwm.passwords["ssid"] == "password");
  CHECK(pwm.scores["ssid"] == 128);
  CHECK(pwm.passwords["coucou"] == "salut");
  CHECK(pwm.scores["coucou"] == 128);
  pwm.save();
  CHECK(preferences.getString("indexes") == "6,5,4,8,");
  CHECK(preferences.getString("buffer") == "coucousalut\x80ssidpassword\x80");
}

TEST_CASE("Should encode three ssid and paswords") {
  PipoPWManager pwm;
  Preferences preferences = pwm.preferences;
  pwm.add("ssid", "password");
  pwm.add("coucou", "salut");
  pwm.add("salut salut", "pwd");
  CHECK(pwm.passwords.size() == 3);
  CHECK(pwm.passwords["ssid"] == "password");
  CHECK(pwm.scores["ssid"] == 128);
  CHECK(pwm.passwords["coucou"] == "salut");
  CHECK(pwm.scores["coucou"] == 128);
  CHECK(pwm.passwords["salut salut"] == "pwd");
  CHECK(pwm.scores["salut salut"] == 128);
  pwm.save();
  CHECK(preferences.getString("indexes") == "6,5,11,3,4,8,");
  CHECK(preferences.getString("buffer") ==
        "coucousalut\x80salut salutpwd\x80ssidpassword\x80");
}

TEST_CASE("Should erase the less popular network") {
  PipoPWManager pwm;
  Preferences preferences = pwm.preferences;
  pwm.add("famous", "pwd");
  pwm.add("used often", "pwd");
  pwm.add("used sometimes", "pwd");
  pwm.add("used sometimes2", "pwd");
  pwm.add("not used", "pwd");
  for (int i = 0; i < 10; i++) {
    pwm.promote("famous");
  }
  for (int i = 0; i < 5; i++) {
    pwm.promote("used often");
  }
  pwm.promote("used sometimes");
  pwm.promote("used sometimes2");
  pwm.add("new-network", "pwd");
  // pwm.add("really not used", "pwd");
  CHECK(pwm.scores.size() == 5);
  bool notUsedErasedS = true;
  bool notUsedErasedP = true;
  bool newNetworkAddedS = false;
  bool newNetworkAddedP = false;
  std::cout << "scores" << std::endl;
  for (auto const& pair : pwm.scores) {
    if (pair.first == "not used") {
      notUsedErasedS = false;
    }
    if (pair.first == "new-network") {
      newNetworkAddedS = true;
    }
  }
  for (auto const& pair : pwm.passwords) {
    if (pair.first == "not used") {
      notUsedErasedP = false;
    }
    if (pair.first == "new-network") {
      newNetworkAddedP = true;
    }
  }

  CHECK(notUsedErasedS == true);
  CHECK(notUsedErasedS == true);
  CHECK(newNetworkAddedP == true);
  CHECK(newNetworkAddedP == true);
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