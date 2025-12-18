#ifndef PIPOPWMANAGER_H
#define PIPOPWMANAGER_H
#include <Arduino.h>
#include <Preferences.h>
#include <map>
#include <string>
#include "HW_CONFIG.h"
#include "utils/debug.h"

class PipoPWManager {
 private:
  bool userWantsDisconnected =
      false;  // Persisted flag for user disconnect request

  /**
   * @brief Normalizes order numbers to maintain relative ordering while keeping values small
   * Resets all order numbers to sequential values (1, 2, 3...) based on current ordering
   * This prevents orderCounter overflow and keeps numbers manageable
   */
  void normalizeOrderNumbers() {
    if (orderNumbers.empty()) {
      orderCounter = 0;
      return;
    }

    // Create sorted list of (ssid, order) pairs
    std::vector<std::pair<std::string, unsigned long>> sorted;
    for (const auto& pair : orderNumbers) {
      sorted.push_back(pair);
    }

    // Sort by order number (ascending)
    std::sort(sorted.begin(), sorted.end(),
              [](const std::pair<std::string, unsigned long>& a,
                 const std::pair<std::string, unsigned long>& b) {
                return a.second < b.second;
              });

    // Reassign sequential order numbers starting from 1
    orderCounter = 0;
    for (const auto& pair : sorted) {
      orderNumbers[pair.first] = ++orderCounter;
    }

    log_d("Normalized order numbers, new orderCounter: %lu", orderCounter);
  }

  /**
   * @brief Validates data integrity after loading from preferences
   * @return true if data is valid, false if corrupted
   */
  bool validateData() {
    // Check password/order consistency
    if (passwords.size() != orderNumbers.size()) {
      log_e("Data corruption: password count %zu != order count %zu",
            passwords.size(), orderNumbers.size());
      return false;
    }

    // Verify all passwords have order numbers
    for (const auto& pair : passwords) {
      if (orderNumbers.find(pair.first) == orderNumbers.end()) {
        log_e("Data corruption: password '%s' has no order number",
              pair.first.c_str());
        return false;
      }
    }

    // Verify all order numbers have passwords
    for (const auto& pair : orderNumbers) {
      if (passwords.find(pair.first) == passwords.end()) {
        log_e("Data corruption: order for '%s' has no password",
              pair.first.c_str());
        return false;
      }
    }

    // Check network count limit
    if (passwords.size() > MAX_NETWORKS) {
      log_e("Data corruption: stored %zu networks exceeds limit %d",
            passwords.size(), MAX_NETWORKS);
      return false;
    }

    return true;
  }

 public:
  PipoPWManager() : orderCounter(0), lastConnectedSSID("") {};

  void setup() {
    preferences.begin("pipo-wifi", false);
    load();
    print_stored_ssids();
    if (DEBUG_HEAP)
      pipoDebugHeap("End setup PipoPWManager");
  };

  void clear() {
    passwords.clear();
    orderNumbers.clear();
    orderCounter = 0;
    lastConnectedSSID = "";
  }

  /**
   * @brief Prints the stored SSIDs and their scores for debugging
   */
  void print_stored_ssids() {
    log_d("Stored SSIDs and order:");
    log_d("  Last connected: %s",
          lastConnectedSSID.empty() ? "(none)" : lastConnectedSSID.c_str());
    for (const auto& pair : passwords) {
      String ssid = String(pair.first.c_str());
      unsigned long order = orderNumbers[pair.first];
      log_d("  SSID: %s, Order: %lu", ssid.c_str(), order);
    }
  }

  /**
  * @brief Saves the passwords to the preferences
  */
  void save() {
    String buffer = "";
    String indexes = "";
    String orders = "";
    for (auto const& pair : passwords) {
      indexes += pair.first.length();
      indexes += ',';
      indexes += pair.second.length();
      indexes += ',';
      buffer += String(pair.first.c_str());
      buffer += String(pair.second.c_str());
      // Store order as text (avoid binary null bytes issue with Arduino String)
      orders += String(orderNumbers[pair.first]);
      orders += ',';
    }
    preferences.putString("indexes", indexes);
    preferences.putString("buffer", buffer);
    preferences.putString("orders", orders);
    preferences.putBool("userDisconnect", userWantsDisconnected);
    preferences.putString("lastSSID", String(lastConnectedSSID.c_str()));
    preferences.putULong("orderCtr", orderCounter);
  };

  /**
  * @brief Loads the passwords from the preferences
  */
  void load() {
    String indexes = preferences.getString("indexes", "");
    String buffer = preferences.getString("buffer", "");
    String orders = preferences.getString("orders", "");
    userWantsDisconnected = preferences.getBool("userDisconnect", false);
    lastConnectedSSID =
        std::string(preferences.getString("lastSSID", "").c_str());
    orderCounter = preferences.getULong("orderCtr", 0);
#ifndef UNIT_TEST
// Add your wifi here for quick debug and wifi setup
#endif
    int n = 0;
    bool isSSID = true;
    int ssidLen = 0;
    int pwdLen = 0;
    int offset = 0;
    int L = indexes.length();

    // Parse indexes and buffer for SSIDs and passwords
    for (int i = 0; i < L; i++) {
      if (indexes[i] != ',') {
        n *= 10;
        n += indexes[i] - '0';
        continue;
      }
      if (isSSID) {
        isSSID = false;
        ssidLen = n;
        n = 0;
      } else {
        pwdLen = n;
        int pwdOffset = offset + ssidLen;
        String ssid = buffer.substring(offset, pwdOffset);
        String password = buffer.substring(pwdOffset, pwdOffset + pwdLen);
        passwords[std::string(ssid.c_str())] = std::string(password.c_str());
        offset += (ssidLen + pwdLen);
        isSSID = true;
        ssidLen = 0;
        pwdLen = 0;
        n = 0;
      }
    }

    // Parse orders (comma-separated list)
    n = 0;
    int orderIndex = 0;
    L = orders.length();
    for (int i = 0; i <= L; i++) {  // <= to process last number
      if (i == L || orders[i] == ',') {
        if (n > 0 || i > 0) {  // Have a valid order number
          // Find corresponding SSID by index
          int currentIndex = 0;
          for (const auto& pair : passwords) {
            if (currentIndex == orderIndex) {
              orderNumbers[pair.first] = n;
              break;
            }
            currentIndex++;
          }
          orderIndex++;
          n = 0;
        }
      } else {
        n = n * 10 + (orders[i] - '0');
      }
    }

    // Validate loaded data and recover from corruption
    if (!validateData()) {
      log_w(
          "Corrupted data detected during load, clearing all stored networks");
      clear();
      save();  // Persist the cleared state to prevent repeated corruption
    }
  }

  /**
  * @brief Adds a ssid and password to the list (does not save it)
  * If network already exists, updates password only (order unchanged)
  * If at capacity, evicts the oldest network (lowest order number)
  */
  void add(String ssid, String password) {
    std::string c_ssid = std::string(ssid.c_str());

    // If network already exists, just update password (keep existing order)
    if (passwords.find(c_ssid) != passwords.end()) {
      passwords[c_ssid] = std::string(password.c_str());
      log_d("Updated password for existing network '%s'", ssid.c_str());
      return;
    }

    // Normalize order numbers periodically to prevent overflow and keep values small
    // Do this before adding to ensure we have room for the new entry
    if (orderCounter > 1000 ||
        (orderCounter > 100 && passwords.size() < MAX_NETWORKS)) {
      normalizeOrderNumbers();
    }

    // FIFO eviction: remove network with lowest order number if at capacity
    if (passwords.size() >= MAX_NETWORKS) {
      unsigned long minOrder = ULONG_MAX;
      std::string oldestSSID = "";
      for (const auto& pair : orderNumbers) {
        if (pair.second < minOrder) {
          minOrder = pair.second;
          oldestSSID = pair.first;
        }
      }
      if (!oldestSSID.empty()) {
        log_i(
            "Network limit reached, evicting oldest network '%s' (order: %lu)",
            oldestSSID.c_str(), minOrder);
        passwords.erase(oldestSSID);
        orderNumbers.erase(oldestSSID);
        if (lastConnectedSSID == oldestSSID) {
          lastConnectedSSID = "";
        }
      }
    }

    // Add new network with next order number
    passwords[c_ssid] = std::string(password.c_str());
    orderNumbers[c_ssid] = ++orderCounter;
    log_d("Added network '%s' with order %lu", ssid.c_str(), orderCounter);
  }

  /**
   * @brief Removes a ssid and password from the list (does not save it)
   * @param ssid the ssid of the network to remove
   */
  void remove(String ssid) {
    std::string c_ssid = std::string(ssid.c_str());
    passwords.erase(c_ssid);
    orderNumbers.erase(c_ssid);
    if (lastConnectedSSID == c_ssid) {
      lastConnectedSSID = "";
    }
  }

  /**
   * @brief Marks a network as connected - updates its order to be most recent and sets as lastConnected
   * @param ssid the ssid of the network that was connected
   */
  void markAsConnected(String ssid) {
    std::string c_ssid = std::string(ssid.c_str());
    if (orderNumbers.find(c_ssid) == orderNumbers.end()) {
      log_w("Attempted to mark unknown network '%s' as connected",
            ssid.c_str());
      return;
    }

    // Update to most recent order
    orderNumbers[c_ssid] = ++orderCounter;
    lastConnectedSSID = c_ssid;
    log_d("Marked '%s' as connected with order %lu", ssid.c_str(),
          orderCounter);
  }

  /**
  * @brief Returns the password for a given ssid
  * @param ssid the ssid of the network
  * @return the password of the network, empty string if not found
  */
  String getPassword(String ssid) {
    return String(passwords[std::string(ssid.c_str())].c_str());
  }
  /**
   * @brief Returns wether we have a password for a given ssid
   * @param ssid the ssid of the network
   * @return true if we have a password for the ssid, false otherwise
   */
  bool hasSSID(String ssid) {
    return passwords.find(std::string(ssid.c_str())) != passwords.end();
  }

  /**
   * @brief Returns the last connected SSID
   * @return the SSID of the last connected network, empty string if none
   */
  String getLastConnectedSSID() { return String(lastConnectedSSID.c_str()); }

  /**
   * @brief Clear user disconnect request (called when user manually connects)
   * Note: Does not save immediately - call save() later from non-interrupt context
   */
  void clearDisconnectRequest() {
    if (userWantsDisconnected) {
      log_i("Clearing user disconnect request");
      userWantsDisconnected = false;
      // Don't save here - will be saved by caller in safe context
    }
  }

  /**
   * @brief Set user disconnect request (called when user clicks disconnect)
   * Note: Does not save immediately - call save() later from non-interrupt context
   */
  void setDisconnectRequest() {
    log_i("Setting user disconnect request");
    userWantsDisconnected = true;
    // Don't save here - will be saved by caller in safe context
  }

  /**
   * @brief Check if board should auto-connect at boot
   * @return false if user has requested disconnect, true otherwise
   */
  bool shouldAutoConnect() { return !userWantsDisconnected; }

  /**
   * @brief Clears all stored networks and resets user preferences (factory reset)
   * Useful for troubleshooting or resetting device to clean state
   */
  void clearAll() {
    log_i("Factory reset: clearing all stored networks and preferences");
    clear();
    userWantsDisconnected = false;
    save();
  }

  /**
   * @brief Maximum number of networks to remember
   */
  const unsigned char MAX_NETWORKS = 5;
  Preferences preferences;
  /**
   * @brief The passwords
   * key: ssid
   * value: password
   */
  std::map<std::string, std::string> passwords;
  /**
   * @brief Order number for each network (for FIFO eviction and tracking last used)
   * Higher values = more recently used/added
   */
  std::map<std::string, unsigned long> orderNumbers;
  /**
   * @brief Counter to generate monotonically increasing order numbers
   */
  unsigned long orderCounter;
  /**
   * @brief SSID of the last successfully connected network
   */
  std::string lastConnectedSSID;
};

#endif  // PIPOPWMANAGER_H
