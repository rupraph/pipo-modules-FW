#include <Arduino.h>
#include "fs_tools.h"
#include <WiFiManager.h> 
#include "midiBLE.h"
#include "midiRtp.h"
#include <ESPAsyncWebServer.h>
#include "dist_sensor.h"
#include "osc_handler.h"
#include "midi_translator.h"
#include "midi_io.h"

AsyncWebServer server(80);
dist_Sensor dist(14,13);
OSC_handler osc;
MidiTranslator midi_translator;
midi_io midiio;

#define FORMAT_LITTLEFS_IF_FAILED true

// unsigned long t0 = millis();
// bool isConnected = false;

void setup(){

    midiio.setup_usb_midi();

    Serial.begin(115200);

    if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
        Serial.println("LittleFS Mount Failed");
        return;
    }
    Serial.println("LittleFS Mount Success");

    
    WiFiManager wm;
    // reset settings - wipe stored credentials for testing
    // wm.resetSettings();
    bool res;
    res = wm.autoConnect("AutoConnectAP","password"); // password protected ap
    delay(2000);

    
    if(!res) {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
    }



    listDir(LittleFS, "/", 2);
    
    midiBLESetup();
    midiRtpSetup(); //-> not working. can't see the device from mac or windows

    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    server.begin();
    dist.init();
    delay(1000);

    osc.setDestIp(IPAddress(192,168,1,71));
    osc.setoutPort(8000);
    osc.start();

    midi_translator.set_Scale_Type("minor");
    midi_translator.printScale(midi_translator.current_scale);
// Load config

// setup sensor

}

int distValue=0;
int midi_note=0;

void loop() {
    //midiUSBLoop();
    //midiRtpLoop();
    //midiBLELoop();
    dist.update();

    //dist.print_last();
    distValue= dist.get_moving_average(5);

    //sendHiResCC(distValue);



    midi_note=midi_translator.get_note(distValue/400.0);
    Serial.print("Dist: ");
    Serial.print(distValue);
    Serial.print(" Midi: ");
    Serial.println(midi_note);
    midiio.sendNoteOn(midi_note,127,1);
    
    osc.sendOscMessage(distValue);
    
    
  // read/update from sensor
  // poll webserver for config change
  // convert sensor to midi
  // send midi
}


    // connect to wifi manually
    // WiFi.mode(WIFI_STA);
    // WiFi.begin("Klurp", "plokplokplok");
    // while (WiFi.status() != WL_CONNECTED) {
    //     delay(500);
    //     Serial.print(".");
    // }
    // Serial.println("Connected to WiFi");
    // Serial.println(WiFi.localIP());