#include "midi_io.h"

midi_io midiio;

void midi_io::setup(const char* deviceName) {
  log_i("midi_io setup started");

  if (DEBUG_HEAP)
    pipoDebugHeap("Start setup midi_io");

#ifndef DISABLE_USB_COMM
  log_i("midiUSBSetup");
  String usbName = String(deviceName) + "-USB";
  MidiUSBSetup(usbName.c_str());
#endif

  log_i("midiBLESetup");
  if (DEBUG_HEAP)
    pipoDebugHeap("MidiBLESetup: start");
#ifdef INCLUDE_BLE
  if (config.general_config["BLEEnabled"]) {
    String bleName = String(deviceName) + "-BLE";
    midiBLESetup(bleName.c_str());
  }
#endif
  log_i("midiBLESetup done");
  if (DEBUG_HEAP)
    pipoDebugHeap("MidiBLESetup: end");
  // midiRtpSetup();
}
// if sustainmil is 0 = infinite sustain from sustain manager
void midi_io::sendNoteOn(int note, int velocity, int channel,
                         unsigned long sustain_ms) {
  unsigned long time = millis();
  if (is_note_playing(note, channel)) {
    // don't play again notes faster than 10ms
    if (channel_note_list[channel][note].starttime + 10 > time) {
      // Serial.println("not playing");
      return;
    }
  }
  MidiUSBsendNoteOn(note, velocity, channel);
#ifdef INCLUDE_BLE
  if (config.general_config["BLEEnabled"]) {
    MidiBLEsendNoteOn(note, velocity, channel);
  }
#endif

  // pipoSocket.sendNoteOn(note, velocity,channel);
  hwui.init_blink_once(SEND_LED, NOTE_BLINK_TIME, NOTE_BLINK_BRIGHTNESS);

  // insert or update note to channel_note_list
  channel_note_list[channel][note].on = true;
  channel_note_list[channel][note].starttime = time;

  //set appropriate sustain endtime (0=infinite sustain)
  if (sustain_ms > 0) {
    channel_note_list[channel][note].sustain_endtime = time + sustain_ms;
  } else {
    channel_note_list[channel][note].sustain_endtime = 0;
  }
}

void midi_io::sendNoteOff(int note, int velocity, int channel) {

  if (channel_note_list[channel].find(note) !=
      channel_note_list[channel].end()) {
    MidiUSBsendNoteOff(note, velocity, channel);
#ifdef INCLUDE_BLE
    if (config.general_config["BLEEnabled"]) {
      MidiBLEsendNoteOff(note, velocity, channel);
    }
#endif
    // pipoSocket.sendNoteOff(note, velocity,channel);
    channel_note_list[channel].erase(note);
  }
}

void midi_io::sendAllNotesOff(int channel) {
  unsigned long time = millis();

  // loop through channel_note_list[channel] and send note off for all notes
  // Create a copy of the keys (notes)
  std::vector<int> notes;
  for (auto const& pair : channel_note_list[channel]) {
    notes.push_back(pair.first);
  }

  // Loop through the notes and send note off for all notes
  for (int note : notes) {
    this->sendNoteOff(note, 127, channel);
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

void midi_io::sendControlChange(int control, int value, int channel,
                                bool hires) {
  if (lastcc[channel][control] != value) {
    if (hires) {
      sendHiResControlChange(control, value, channel);
    } else {
      MidiUSBsendCC(control, value, channel);
#ifdef INCLUDE_BLE
      if (config.general_config["BLEEnabled"]) {
        MidiBLEsendCC(control, value, channel);
      }
#endif
    }
    lastcc[channel][control] = value;
    hwui.init_blink_once(SEND_LED, NOTE_BLINK_TIME, NOTE_BLINK_BRIGHTNESS);
  }
}

void midi_io::sendHiResControlChange(int control, int value, int channel) {
  int sizeddata = value & 0x3FFF;
  int msb = (sizeddata >> 7) & 0x7F;
  int lsb = sizeddata & 0x7F;

  MidiUSBsendCC(control, msb, channel);
  MidiUSBsendCC(control + 32, lsb, channel);
  // MidiUsb.sendControlChange(control, value, channel);
  hwui.init_blink_once(SEND_LED, NOTE_BLINK_TIME, NOTE_BLINK_BRIGHTNESS);
}

bool midi_io::is_note_playing(int note, int channel) {
  bool res =
      channel_note_list[channel].find(note) != channel_note_list[channel].end();
  return res;
}

void midi_io::manage_sustain() {
  for (int i = 0; i < 16; i++) {
    // Create a copy of the keys (notes)
    std::vector<int> notes;
    for (auto const& pair : channel_note_list[i]) {
      notes.push_back(pair.first);
    }

    // Loop through the notes and send note off for all notes
    for (int note : notes) {
      // 0 means infinite sustain (no note off after a duration, off only by sensor/engine sending off)
      if (channel_note_list[i][note].sustain_endtime != 0) {
        if (channel_note_list[i][note].sustain_endtime < millis()) {
          sendNoteOff(note, 127, i);
        }
      }
    }
  }
}

void midi_io::printNoteList(int channel) {
  log_d("Printing note list");
  for (auto const& pair : channel_note_list[channel]) {
    log_d("Note: %d isplaying ?: %d", pair.first, 
          is_note_playing(pair.first, channel));
  }
}
