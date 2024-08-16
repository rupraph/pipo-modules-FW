#include "wifi_tools.h"

WiFiManager wm;



void setup_wifi(){
    // setup wifi through wifi manager
    if (config.general_config["Wifi_mode"] == "AP")
    {
        delay(1000);
        Serial.println("Starting AP mode");
        WiFi.softAP("Pipo", "pipo1234");
    }
    else{
    Serial.println("Starting STA mode");


    WiFi.mode(WIFI_STA);

    // WiFiManager wm;
    wm.setDarkMode(true);
    wm.setHostname(string(PIPO_TYPE).c_str());
    wm.setConfigPortalBlocking(false);
    wm.setBreakAfterConfig(true);
    wm.setDebugOutput(true);
    wm.setDebugOutput(true);
    wm.setWiFiAutoReconnect(true);
    wm.setCleanConnect(true);
    wm.setSaveConfigCallback([]() {
        ESP.restart();
    });

    if(digitalRead(MODE_SW)==LOW){
        delay(3000);
    }
    // keep pressing to reset

    ///////// HIGH here should be low. temporary patch to cope with switch not wired corectly)
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
    wm.process();

    if (WiFi.status() == WL_CONNECTED && !hwui.is_pulsing(WIFI_LED))
    {
        Serial.println("Wifi connected");
        hwui.start_pulse(WIFI_LED, 3000, 3, 30);

        //Todo: Starting the server here does not seem to work.
        // for now, when setting wifi for first time, then reset and server works.
        //server cannot be setup in the loop ?

        // Serial.println("Wifi connected, starting config page");
        // wm.setDisableConfigPortal(true);
    }

    if (Serial.available() > 0)
    {
        char c = Serial.read();
        if (c == 'r')
        {
            Serial.println("Resetting wifi");
            wm.resetSettings();
            ESP.restart();
        }
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
    WiFi.begin("ssid", "password");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print("try connect to hardcoded wifi");
    }
    Serial.println("Connected to WiFi");
    Serial.println(WiFi.localIP());
}