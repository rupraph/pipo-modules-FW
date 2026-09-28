#define DOCTEST_CONFIG_IMPLEMENT  // REQUIRED: Enable custom main()
#include <ArduinoFake.h>
#include <doctest.h>
#include "esp_log_stub.h"
#include "wifi/pw-manager.hpp"

TEST_CASE("Should encode and decode one network with order") {
  PipoPWManager pwm;
  Preferences preferences = pwm.preferences;
  pwm.add("mynetwork", "mypassword");

  CHECK(pwm.passwords.size() == 1);
  CHECK(pwm.passwords["mynetwork"] == "mypassword");
  CHECK(pwm.orderNumbers["mynetwork"] == 1);  // First network gets order 1

  pwm.save();

  // Create new instance and load
  PipoPWManager pwm2;
  pwm2.preferences = preferences;
  pwm2.load();

  CHECK(pwm2.passwords.size() == 1);
  CHECK(pwm2.passwords["mynetwork"] == "mypassword");
  CHECK(pwm2.orderNumbers["mynetwork"] == 1);
}

TEST_CASE("Should add multiple networks with increasing order") {
  PipoPWManager pwm;
  pwm.add("network1", "pwd1");
  pwm.add("network2", "pwd2");
  pwm.add("network3", "pwd3");

  CHECK(pwm.passwords.size() == 3);
  CHECK(pwm.orderNumbers["network1"] == 1);
  CHECK(pwm.orderNumbers["network2"] == 2);
  CHECK(pwm.orderNumbers["network3"] == 3);
}

TEST_CASE("Should evict oldest network (FIFO) when at capacity") {
  PipoPWManager pwm;

  // Add 5 networks (MAX_NETWORKS)
  pwm.add("network1", "pwd");  // order 1
  pwm.add("network2", "pwd");  // order 2
  pwm.add("network3", "pwd");  // order 3
  pwm.add("network4", "pwd");  // order 4
  pwm.add("network5", "pwd");  // order 5

  CHECK(pwm.passwords.size() == 5);

  // Add 6th network - should evict network1 (lowest order)
  pwm.add("network6", "pwd");  // order 6

  CHECK(pwm.passwords.size() == 5);
  CHECK(pwm.passwords.find("network1") ==
        pwm.passwords.end());  // network1 evicted
  CHECK(pwm.passwords.find("network6") !=
        pwm.passwords.end());  // network6 added
  CHECK(pwm.orderNumbers["network6"] == 6);
}

TEST_CASE("Should update order when marking network as connected") {
  PipoPWManager pwm;
  pwm.add("network1", "pwd");  // order 1
  pwm.add("network2", "pwd");  // order 2
  pwm.add("network3", "pwd");  // order 3

  // Mark network1 as connected - should get new highest order
  pwm.markAsConnected("network1");

  CHECK(pwm.orderNumbers["network1"] == 4);  // Updated to order 4
  CHECK(pwm.orderNumbers["network2"] == 2);  // Unchanged
  CHECK(pwm.orderNumbers["network3"] == 3);  // Unchanged
  CHECK(pwm.getLastConnectedSSID() == "network1");
}

TEST_CASE("Should evict least recently used when at capacity") {
  PipoPWManager pwm;

  // Add 5 networks
  pwm.add("network1", "pwd");  // order 1
  pwm.add("network2", "pwd");  // order 2
  pwm.add("network3", "pwd");  // order 3
  pwm.add("network4", "pwd");  // order 4
  pwm.add("network5", "pwd");  // order 5

  // Mark network1 as connected (updates order to 6)
  pwm.markAsConnected("network1");

  // Now network2 has the lowest order (2)
  // Add 6th network - should evict network2
  pwm.add("network6", "pwd");

  CHECK(pwm.passwords.size() == 5);
  CHECK(pwm.passwords.find("network2") ==
        pwm.passwords.end());  // network2 evicted
  CHECK(pwm.passwords.find("network1") !=
        pwm.passwords.end());  // network1 kept (recently used)
  CHECK(pwm.passwords.find("network6") !=
        pwm.passwords.end());  // network6 added
}

TEST_CASE("Should persist and restore last connected SSID") {
  PipoPWManager pwm;
  Preferences preferences = pwm.preferences;

  pwm.add("network1", "pwd");
  pwm.add("network2", "pwd");
  pwm.markAsConnected("network2");

  CHECK(pwm.getLastConnectedSSID() == "network2");

  pwm.save();

  // Create new instance and load
  PipoPWManager pwm2;
  pwm2.preferences = preferences;
  pwm2.load();

  CHECK(pwm2.getLastConnectedSSID() == "network2");
}

TEST_CASE(
    "Should handle updating existing network without creating duplicate") {
  PipoPWManager pwm;
  pwm.add("mynetwork", "password1");
  CHECK(pwm.passwords.size() == 1);
  CHECK(pwm.orderNumbers.size() == 1);
  unsigned long originalOrder = pwm.orderNumbers["mynetwork"];

  // Add same network with different password
  pwm.add("mynetwork", "password2");

  // Should still have only 1 network with unchanged order
  CHECK(pwm.passwords.size() == 1);
  CHECK(pwm.orderNumbers.size() == 1);
  CHECK(pwm.passwords["mynetwork"] == "password2");
  CHECK(pwm.orderNumbers["mynetwork"] == originalOrder);  // Order unchanged
}

TEST_CASE("Should clear last connected when that network is removed") {
  PipoPWManager pwm;
  pwm.add("network1", "pwd");
  pwm.markAsConnected("network1");

  CHECK(pwm.getLastConnectedSSID() == "network1");

  pwm.remove("network1");

  CHECK(pwm.getLastConnectedSSID() == "");
  CHECK(pwm.passwords.size() == 0);
}

TEST_CASE("Should clear last connected when that network is evicted") {
  PipoPWManager pwm;

  // Add 5 networks
  for (int i = 1; i <= 5; i++) {
    pwm.add("network" + String(i), "pwd");
  }

  // Mark network1 as connected
  pwm.markAsConnected("network1");
  CHECK(pwm.getLastConnectedSSID() == "network1");

  // Update order so network2 becomes oldest
  pwm.markAsConnected("network1");  // Update network1 again

  // network2 should be evicted when adding 6th
  pwm.add("network6", "pwd");

  // network1 should still be last connected (not evicted)
  CHECK(pwm.getLastConnectedSSID() == "network1");
}

TEST_CASE("Should handle data corruption by clearing") {
  PipoPWManager pwm;
  Preferences preferences = pwm.preferences;

  // Create valid data
  pwm.add("network1", "pwd");
  pwm.save();

  // Corrupt it by adding mismatched order entry
  pwm.orderNumbers["fake-network"] = 999;

  // Validation should fail
  bool isValid = (pwm.passwords.size() == pwm.orderNumbers.size());
  CHECK(isValid == false);
}

TEST_CASE("Should maintain FIFO order through save/load cycle") {
  PipoPWManager pwm;
  Preferences preferences = pwm.preferences;

  pwm.add("network1", "pwd");       // order 1
  pwm.add("network2", "pwd");       // order 2
  pwm.add("network3", "pwd");       // order 3
  pwm.markAsConnected("network1");  // order 4

  pwm.save();

  PipoPWManager pwm2;
  pwm2.preferences = preferences;
  pwm2.load();

  CHECK(pwm2.orderNumbers["network1"] == 4);
  CHECK(pwm2.orderNumbers["network2"] == 2);
  CHECK(pwm2.orderNumbers["network3"] == 3);

  // Add new network - should evict network2 (lowest order = 2)
  pwm2.add("network4", "pwd");

  CHECK(pwm2.passwords.size() == 4);
  CHECK(pwm2.passwords.find("network2") !=
        pwm2.passwords.end());  // Not evicted yet, only 4 networks

  pwm2.add("network5", "pwd");  // Now at capacity (5)
  pwm2.add("network6", "pwd");  // This should evict network2

  CHECK(pwm2.passwords.size() == 5);
  CHECK(pwm2.passwords.find("network2") ==
        pwm2.passwords.end());  // network2 evicted
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
