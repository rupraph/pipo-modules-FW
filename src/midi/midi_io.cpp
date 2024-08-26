#include "midi_io.h"



void midi_io::setup()
{
    MidiUSBSetup();
    midiBLESetup();
    //midiRtpSetup();
}
// if sustainmil is 0 it will not send a note off
void midi_io::sendNoteOn(int note, int velocity, int channel,unsigned long sustain_mil)
{
        MidiUSBsendNoteOn(note, velocity, channel);   
        MidiBLEsendNoteOn(note, velocity, channel);
        midisocket.sendNoteOn(note, velocity,channel);
        hwui.init_blink_once(SEND_LED, NOTE_BLINK_TIME, NOTE_BLINK_BRIGHTNESS);
        // insert or update note to channel_note_list
        channel_note_list[channel][note]={true, millis()+sustain_mil};
}

void midi_io::sendNoteOff(int note, int velocity, int channel)
{

        if (channel_note_list[channel].find(note) != channel_note_list[channel].end())
        {
            MidiUSBsendNoteOff(note, velocity, channel);
            MidiBLEsendNoteOff(note, velocity, channel);
            midisocket.sendNoteOff(note, velocity,channel);
            channel_note_list[channel].erase(note);
        }
        
}

void midi_io::sendAllNotesOff(int channel)
{

    //loop through channel_note_list[channel] and send note off for all notes
   // Create a copy of the keys (notes)
    std::vector<int> notes;
    for (auto const& pair : channel_note_list[channel])
    {
        notes.push_back(pair.first);
    }

    // Loop through the notes and send note off for all notes
    for (int note : notes)
    {
        this->sendNoteOff(note, 127, channel);
    }
}

void midi_io::sendControlChange(int control, int value, int channel, bool hires)
{
    if (lastcc[channel][control]!=value)
    {
        if (hires)
        {
            sendHiResControlChange(control, value, channel);
        }
        else
        {
            MidiUSBsendCC(control, value, channel);
            MidiBLEsendCC(control, value, channel);
        }
        lastcc[channel][control]=value;
        hwui.init_blink_once(SEND_LED, NOTE_BLINK_TIME, NOTE_BLINK_BRIGHTNESS);
    }
        

}

void midi_io::sendHiResControlChange(int control, int value, int channel)
{
    int sizeddata=value & 0x3FFF;
    int msb=(sizeddata>>7) & 0x7F;
    int lsb=sizeddata & 0x7F;

    MidiUSBsendCC(control, msb, channel);
    MidiUSBsendCC(control+32, lsb, channel);
    //MidiUsb.sendControlChange(control, value, channel);
    hwui.init_blink_once(SEND_LED, NOTE_BLINK_TIME, NOTE_BLINK_BRIGHTNESS);
}


void midi_io::manage_sustain()
{
    for (int i=0; i<16; i++)
    {
        // Create a copy of the keys (notes)
        std::vector<int> notes;
        for (auto const& pair : channel_note_list[i])
        {
            notes.push_back(pair.first);
        }

        // Loop through the notes and send note off for all notes
        for (int note : notes)
        {
            if (channel_note_list[i][note].sustain_endtime < millis())
            {
                sendNoteOff(note, 127, i);
            }
        }
    }

}

