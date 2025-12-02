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
  log_i("MIDI RTP connected");
  isConnected = true;
}

void OnDisconnected(const APPLEMIDI_NAMESPACE::ssrc_t& ssrc) {
  log_i("MIDI RTP disconnected");
  isConnected = false;
}

void midiRtpSetup() {
  if (!MDNS.begin(AppleMidiRtp.getName())) {
    log_e("Error setting up MDNS responder for MIDI RTP!");
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
    // delay(1000);
  }
}

void OnAppleMidiException(const APPLEMIDI_NAMESPACE::ssrc_t& ssrc,
                          const APPLEMIDI_NAMESPACE::Exception& e,
                          const int32_t value) {
  switch (e) {
    case APPLEMIDI_NAMESPACE::Exception::BufferFullException:
      log_e("MIDI RTP: BufferFullException");
      break;
    case APPLEMIDI_NAMESPACE::Exception::ParseException:
      log_e("MIDI RTP: ParseException");
      break;
    case APPLEMIDI_NAMESPACE::Exception::TooManyParticipantsException:
      log_e("MIDI RTP: TooManyParticipantsException");
      break;
    case APPLEMIDI_NAMESPACE::Exception::UnexpectedInviteException:
      log_e("MIDI RTP: UnexpectedInviteException");
      break;
    case APPLEMIDI_NAMESPACE::Exception::ParticipantNotFoundException:
      log_e("MIDI RTP: ParticipantNotFoundException");
      break;
    case APPLEMIDI_NAMESPACE::Exception::ComputerNotInDirectory:
      log_e("MIDI RTP: ComputerNotInDirectory");
      break;
    case APPLEMIDI_NAMESPACE::Exception::NotAcceptingAnyone:
      log_e("MIDI RTP: NotAcceptingAnyone");
      break;
    case APPLEMIDI_NAMESPACE::Exception::ListenerTimeOutException:
      log_e("MIDI RTP: ListenerTimeOutException");
      break;
    case APPLEMIDI_NAMESPACE::Exception::MaxAttemptsException:
      log_e("MIDI RTP: MaxAttemptsException");
      break;
    case APPLEMIDI_NAMESPACE::Exception::
        NoResponseFromConnectionRequestException:
      log_e("MIDI RTP: No response from connection request - check address, port, and firewall");
      break;
    case APPLEMIDI_NAMESPACE::Exception::SendPacketsDropped:
      log_w("MIDI RTP: SendPacketsDropped");
      break;
    case APPLEMIDI_NAMESPACE::Exception::ReceivedPacketsDropped:
      log_w("MIDI RTP: ReceivedPacketsDropped");
      break;
  }
}