#include "wifi_tools.h"

WiFiManager wm;

void setup_wifi(){
    // setup wifi through wifi manager
    if (false && config.general_config["Wifi_mode"] == "AP")
    {
        Serial.println("Starting AP mode");
        WiFi.softAP("Pipo", "pipo1234");
    }
    else{
    return debug_connect();
    WiFi.mode(WIFI_STA);

    // WiFiManager wm;
    wm.setDarkMode(true);
    wm.setConfigPortalBlocking(false);
    wm.setDebugOutput(true);
    wm.setWiFiAutoReconnect(true);
    wm.setCleanConnect(true);

    if(digitalRead(MODE_SW)==LOW){
        delay(3000);
    }
    // keep pressing to reset

    ///////// HIGH here should be low. temp patch to cope with switch not wired corectly)
    if (digitalRead(MODE_SW)==HIGH && digitalRead(PP_SW)==LOW)
    {
        Serial.println("Settings reset");
        wm.resetSettings();
        //Setting reset should be mover somewhere else
        // Serial.println("Launching config portal");
        // wm.setConfigPortalBlocking(true);
        // wm.autoConnect("Pipo");
        ESP.restart();
    }
    else{
        if(wm.autoConnect("Pipo")){
            Serial.println("connected...yeey :)");
            //hwui.set_led(WIFI_LED,60);
            hwui.start_pulse(WIFI_LED, 3000, 3, 30);
        }
        else {
            Serial.println("Could not connect automatically, Configportal running");
        }
    }
    }
}

void monitor_wifi(bool is_server_runing){
    // monitor wifi status
    if (WiFi.status() == WL_CONNECTED && !is_server_runing)
    {
        Serial.println("Wifi connected");
        hwui.start_pulse(WIFI_LED, 3000, 3, 30);

        //Todo: Starting the server here does not seem to work.
        // for now, when setting wifi for first time, then reset and server works.
        //server cannot be setup in the loop ?

        // Serial.println("Wifi connected, starting config page");
        // wm.setDisableConfigPortal(true);
    }
    else if(WiFi.status() != WL_CONNECTED){
        //Serial.println("Wifi disconnected");
        hwui.stop_pulse(WIFI_LED);
    }
}

void debug_connect(){
    //temp function for debug
    //connect to wifi manually
    WiFi.mode(WIFI_STA);
    WiFi.begin("4G-Gateway-1B52", "9NG4AT1NARF");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected to WiFi");
    Serial.println(WiFi.localIP());
}