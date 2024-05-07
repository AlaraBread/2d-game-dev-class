#ifndef __ALARA_MIDI__
#define __ALARA_MIDI__

#define MIDI_BUFFER_SIZE 128

#define NOTE_OFF 0x8
#define NOTE_ON 0x9
#define POLY_AFTERTOUCH 0xA
#define CONTROL_CHANGE 0xB
#define PROGRAM_CHANGE 0xC
#define AFTERTOUCH 0xD
#define PITCH_WHEEL 0xE

void midi_select();
void midi_end();
void midi_poll();

#endif