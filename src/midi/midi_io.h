#ifndef MIDI_IO_H
#define MIDI_IO_H

#include "HW_CONFIG.h"
#ifdef INCLUDE_BLE
#include "midi/midiBLE.h"
#endif
// #include "midi/midiRtp.h"
#include "midi/midiUSB.h"
#include <vector>
#include <unordered_map>
#include "utils/hw_ui.h"
#include "server/pipo_socket.h"
using namespace std;

// midiio deals with sending the midi messages to the various output interfaces
// midiio also manages the active notes, and the sustain of the notes

class midi_io {
 public:
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

  // this should be taken care differently since this takes a significant amount of memory
  //should likley be in engine
  uint8_t lastcc[16][128];

  bool off_before_next_note = false;
  // set if off previous note before sending note

  void setup();

  void sendNoteOn(int note, int velocity, int channel,
                  unsigned long sustainmil = 0);
  void sendNoteOff(int note, int velocity, int channel);
  void sendAllNotesOff(int channel);
  void manage_sustain();
  bool is_note_playing(int note, int channel);

  void sendControlChange(int control, int value, int channel,
                         bool hires = false);
  void sendProgramChange(int program, int channel);
  void sendPitchBend(int value, int channel);
  void sendAfterTouch(int pressure, int channel);

  void sendHiResControlChange(int control, int value, int channel);

  //utility functions
  void printNoteList(int channel);
};

extern midi_io midiio;

#endif  //MIDI_IO_H