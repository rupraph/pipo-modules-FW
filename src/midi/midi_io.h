#ifndef MIDI_IO_H
#define MIDI_IO_H

#include <unordered_map>
#include <vector>
#include "hw_ui.h"
#include "midi/midiBLE.h"
#include "midi/midiRtp.h"
#include "midi/midiUSB.h"
#include "server/pipo_socket.h"
using namespace std;

// midiio will deal with sending the midi messages to the various output
// interfaces midiio will also manage the active notes, and the sustain of the
// notes

class midi_io {
 private:
 public:
  // midi_io();
  bool midi_usb_enabled = true;
  bool midi_ble_connected = false;
  bool midi_rtp_connected = false;

  struct NoteSatus {
    bool on;
    unsigned long sustain_endtime;
    unsigned long starttime;
  };
  typedef unordered_map<int, NoteSatus> Notelist;
  Notelist channel_note_list[16];

  // this should be replaced by dynamic allocation since this takes a
  // significant amount of memory
  // should likley be in engine
  uint8_t lastcc[16][128];

  bool off_before_next_note =
      false;  // sends off previous note before sending note

  void setup();
  void sendNoteOn(int note, int velocity, int channel,
                  unsigned long sustainmil = 0);
  void sendNoteOff(int note, int velocity, int channel);
  void sendAllNotesOff(int channel);
  void sendControlChange(int control, int value, int channel,
                         bool hires = false);
  void sendProgramChange(int program, int channel);
  void sendPitchBend(int value, int channel);
  void sendAfterTouch(int pressure, int channel);
  // void update();
  void manage_sustain();
  void sendHiResControlChange(int control, int value, int channel);

  bool is_note_playing(int note, int channel);

 private:
};

#endif  // MIDI_IO_H