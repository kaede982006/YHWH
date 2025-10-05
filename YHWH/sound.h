#ifndef YHWH_SOUND
#define YHWH_SOUND

#include "type.h"

typedef struct {
    int duration;
    int hz;
    Y_BYTE* buffer;
} S_YHWH_SOUND;
S_YHWH_SOUND* sound_payload(int duration);
DWORD WINAPI default_sound_maker(S_YHWH_SOUND s_yhwh_sound);

#endif