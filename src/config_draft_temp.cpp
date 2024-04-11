// this will implement the methods from config.h


#include "config.h"
#include <Preferences.h>


Config::Config() {
    load_config();
}

void Config::load_config() {
    Preferences preferences;
    preferences.begin("config", false);
   for (int i = 0; i < sizeof(configuration); i++) {
        ((uint8_t*)&current_CONFIG)[i] = preferences.getUInt("config", ((uint8_t*)&default_CONFIG)[i]);
    }
    preferences.end();
}