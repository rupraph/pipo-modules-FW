#include "wifi_tools.h"

WiFiManager wm;

// uint8_t broadcastAddress[] = {0x64,0xE8,0x33,0x60,0x7B,0x98}; //send to pipo_motion address
uint8_t destAddress[] = {0x64, 0xE8, 0x33, 0x60, 0x44, 0xF8}; // pipo range address 
esp_now_peer_info_t peerInfo;

// ESP now: there is a arduino API for it, but seems like the default one is ok
// if using wifi for other stuff, channel of esp now must be the same.
// can detect slaves by getting their wifi names.

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
            Serial.print("Channel: ");
            Serial.println(WiFi.channel());
            //hwui.set_led(WIFI_LED,60);
            hwui.start_pulse(WIFI_LED, 3000, 3, 30);
        }
        else {
            Serial.println("Could not connect automatically, Configportal running");
        }
    }
    }
    Serial.println(WiFi.macAddress());
}

#if defined (PIPO_RANGE)
// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
    char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print("Last Packet Recv from: "); Serial.println(macStr);
  float recData;
  memcpy(&recData, incomingData, sizeof(recData));
  Serial.print("Last Packet Recv Data: "); Serial.println(*incomingData);
  Serial.println(recData);
}
#endif

#if defined (PIPO_MOTION)
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
#endif

void setup_esp_now(){
    

    // setup esp now
    if (esp_now_init() == ESP_OK)
    {
        Serial.println("ESP Now Init Success");
    }
    else
    {
        Serial.println("ESP Now Init Failed");
        return;
    }

    #if defined (PIPO_MOTION) //sender 
    
    
    // Create a struct_message called myData
    
    esp_now_register_send_cb(OnDataSent);
    peerInfo.channel = 1; // channel should be the same as wifi connection  
    peerInfo.encrypt = false;
    // Register peer
    memcpy(peerInfo.peer_addr, destAddress, 6);
    
    
    Serial.print("Slave Status: ");
    // check if the peer exists
    bool exists = esp_now_is_peer_exist(peerInfo.peer_addr);
    if ( exists) {
      // Slave already paired.
      Serial.println("Already Paired");
      return ;
    } else {
      // Slave not paired, attempt pair
      esp_err_t addStatus = esp_now_add_peer(&peerInfo);
      if (addStatus == ESP_OK) {
        // Pair success
        Serial.println("Pair success");
        return ;
      } else if (addStatus == ESP_ERR_ESPNOW_NOT_INIT) {
        // How did we get so far!!
        Serial.println("ESPNOW Not Init");
        return ;
      } else if (addStatus == ESP_ERR_ESPNOW_ARG) {
        Serial.println("Invalid Argument");
        return ;
      } else if (addStatus == ESP_ERR_ESPNOW_FULL) {
        Serial.println("Peer list full");
        return ;
      } else if (addStatus == ESP_ERR_ESPNOW_NO_MEM) {
        Serial.println("Out of memory");
        return ;
      } else if (addStatus == ESP_ERR_ESPNOW_EXIST) {
        Serial.println("Peer Exists");
        return ;
      } else {
        Serial.println("Not sure what happened");
        return ;
      }
    }
    Serial.println("Sender setup done");
    #endif
    #if defined (PIPO_RANGE) //receiver
    // Once ESPNow is successfully Init, we will register for recv CB to
    // get recv packer info
    esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
    Serial.println("Receiver setup done");
    #endif

    Serial.println("ESP Now setup done");
}
void esp_now_update(Sensor &sensor){
    float data= sensor.get_value("pitch");
    esp_err_t result = esp_now_send(destAddress, (uint8_t *) &data, sizeof(data));
 
    if (result == ESP_OK) {
    Serial.println("Sent with success");
    }
    else {
    Serial.println("Error sending the data");
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
        // not tested yet
        if (c=='a')
        {
            Serial.println("Switching to AP mode");
            config.general_config["Wifi_mode"] = "AP";
            config.save(config.filename);
            ESP.restart();
        }
        if (c=='a')
        {
            Serial.println("Switching to STA mode");
            config.general_config["Wifi_mode"] = "STA";
            config.save(config.filename);
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