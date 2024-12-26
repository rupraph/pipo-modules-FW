#ifndef PIPOPWMANAGER_H
#define PIPOPWMANAGER_H
#include <Arduino.h>
#ifdef UNIT_TEST
#include <Preferences.hpp>
#else
#include <Preferences.h>
#endif
#include <map>
#include <string>

class PipoPWManager {

 public:
  PipoPWManager(){};

  void setup() {
    preferences.begin("pipo-wifi", false);
    load();
  };

  /**
  * @brief Saves the passwords to the preferences
  */
  void save() {
    String buffer = "";
    String indexes = "";
    for (auto const& pair : passwords) {
      indexes += pair.first.length();
      indexes += ',';
      indexes += pair.second.length();
      indexes += ',';
      buffer += String(pair.first.c_str());
      buffer += String(pair.second.c_str());
      buffer += (char)scores[pair.first];
    }
    preferences.putString("indexes", indexes);
    preferences.putString("buffer", buffer);
  };

  /**
  * @brief Loads the passwords from the preferences
  */
  void load() {
    String indexes = preferences.getString("indexes", "");
    String buffer = preferences.getString("buffer", "");
#ifndef UNIT_TEST
// Add your wifi here for quick debug and wifi setup
#endif
    int n = 0;
    bool isSSID = true;
    int ssidLen = 0;
    int pwdLen = 0;
    int offset = 0;
    int L = indexes.length();
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
        scores[std::string(ssid.c_str())] = buffer[pwdOffset + pwdLen];
        offset += (ssidLen + pwdLen + 1);
        isSSID = true;
        ssidLen = 0;
        pwdLen = 0;
        n = 0;
      }
    }
  }

  /**
  * @brief Adds a ssid and password to the list (do not saves it)
  */
  void add(String ssid, String password) {
    std::string c_ssid = std::string(ssid.c_str());

    if (passwords.size() == MAX_NETWORKS) {
      unsigned char minScore = MAX_SCORE;
      std::string minSSID = "";
      for (auto const& pair : scores) {
        if (pair.second > minScore)
          continue;
        minScore = pair.second;
        minSSID = pair.first;
      }
      passwords.erase(minSSID);
      scores.erase(minSSID);
    }
    passwords[c_ssid] = std::string(password.c_str());
    // check if score exists already
    if (scores.find(c_ssid) == scores.end())
      return;
    scores[c_ssid] = BASE_SCORE;
  }

  /**
   * @brief Updates the score of a given ssid
   * @param ssid the ssid of the network
   * 
   */
  void promote(String ssid) {
    std::string c_ssid = std::string(ssid.c_str());
    if (scores.find(c_ssid) == scores.end()) {
      return;
    }
    unsigned char score = scores[std::string(ssid.c_str())];
    // downgrade all other scores
    for (auto& pair : scores) {
      if (pair.first == c_ssid) {
        pair.second =
            std::min(MAX_SCORE, (unsigned char)(pair.second + MAX_NETWORKS));
      } else {
        pair.second = std::max(MIN_SCORE, (unsigned char)(pair.second - 1));
      }
    }
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
   * @brief Min possible score for a network, cannot be 0 because of string encoding
   */
  const unsigned char MIN_SCORE = 1;
  const unsigned char MAX_SCORE = 255;
  const unsigned char BASE_SCORE = 128;
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
   * @brief score for each wifi (higher is more used) 
   */
  std::map<std::string, unsigned char> scores;
};

#endif  // PIPOPWMANAGER_H
