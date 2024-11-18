#include "midiRtp.h"
#include <WiFi.h>
// #include <WiFiClient.h>
// #include <WiFiUdp.h>
#include <ESPmDNS.h>

bool isConnected = false;

APPLEMIDI_CREATE_INSTANCE(WiFiUDP, MidiRtp, "PipoMotion", DEFAULT_CONTROL_PORT);
// APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE();

// see
// https://github.com/lathoub/Arduino-AppleMIDI-Library/tree/master/examples/wESP32_NoteOnOffEverySec

void OnConnected(const APPLEMIDI_NAMESPACE::ssrc_t& ssrc, const char* name) {
  Serial.println("Rtp Connected");
  isConnected = true;
}

void OnDisconnected(const APPLEMIDI_NAMESPACE::ssrc_t& ssrc) {
  Serial.println("Rtp Disconnected!");
  isConnected = false;
}

void midiRtpSetup() {
  if (!MDNS.begin(AppleMidiRtp.getName())) {
    Serial.println("Error setting up MDNS responder!");
  }

  MidiRtp.begin();
  AppleMidiRtp.setHandleConnected(OnConnected);
  AppleMidiRtp.setHandleDisconnected(OnDisconnected);
  AppleMidiRtp.setHandleException(OnAppleMidiException);

  MDNS.addService("apple-midi", "udp", AppleMidiRtp.getPort());

  Serial.print(AppleMidiRtp.getPort());
}

void midiRtpLoop() {
  MidiRtp.read();
  if (isConnected) {
    MidiRtp.sendNoteOn(69, 127, 1);
    // delay(100);
    // Serial.println("Rtp Note on");
    delay(1000);
  }
}

void OnAppleMidiException(const APPLEMIDI_NAMESPACE::ssrc_t& ssrc,
                          const APPLEMIDI_NAMESPACE::Exception& e,
                          const int32_t value) {
  switch (e) {
    case APPLEMIDI_NAMESPACE::Exception::BufferFullException:
      Serial.println("*** BufferFullException");
      break;
    case APPLEMIDI_NAMESPACE::Exception::ParseException:
      Serial.println("*** ParseException");
      break;
    case APPLEMIDI_NAMESPACE::Exception::TooManyParticipantsException:
      Serial.println("*** TooManyParticipantsException");
      break;
    case APPLEMIDI_NAMESPACE::Exception::UnexpectedInviteException:
      Serial.println("*** UnexpectedInviteException");
      break;
    case APPLEMIDI_NAMESPACE::Exception::ParticipantNotFoundException:
      Serial.println("*** ParticipantNotFoundException" + String(value));
      break;
    case APPLEMIDI_NAMESPACE::Exception::ComputerNotInDirectory:
      Serial.println("*** ComputerNotInDirectory" + String(value));
      break;
    case APPLEMIDI_NAMESPACE::Exception::NotAcceptingAnyone:
      Serial.println("*** NotAcceptingAnyone" + String(value));
      break;
    case APPLEMIDI_NAMESPACE::Exception::ListenerTimeOutException:
      Serial.println("*** ListenerTimeOutException");
      break;
    case APPLEMIDI_NAMESPACE::Exception::MaxAttemptsException:
      Serial.println("*** MaxAttemptsException");
      break;
    case APPLEMIDI_NAMESPACE::Exception::
        NoResponseFromConnectionRequestException:
      Serial.println(
          "***:yyy did't respond to the connection request. Check the address "
          "and port, and any firewall or router settings. (time)");
      break;
    case APPLEMIDI_NAMESPACE::Exception::SendPacketsDropped:
      Serial.println("*** SendPacketsDropped" + String(value));
      break;
    case APPLEMIDI_NAMESPACE::Exception::ReceivedPacketsDropped:
      Serial.println("*** ReceivedPacketsDropped" + String(value));
      break;
  }
}