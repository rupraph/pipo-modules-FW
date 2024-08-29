#ifndef MIDIRTP_H
#define MIDIRTP_H
#define USE_EXT_CALLBACKS
#include <AppleMIDI.h>

void midiRtpSetup();
void midiRtpLoop();
void OnAppleMidiException(const APPLEMIDI_NAMESPACE::ssrc_t&, const APPLEMIDI_NAMESPACE::Exception&, const int32_t);


#endif //MIDIRTP_H