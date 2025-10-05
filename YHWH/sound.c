#include "YHWH.h"
/////////////////////////////////////////////
Y_BYTE* sound1(int duration, int hz) {
    Y_BYTE* buffer = (Y_BYTE*)malloc(hz * duration * sizeof(Y_BYTE));

    for (int t = 0; t < hz * duration; t++) {
        buffer[t] = t * (t >> 4);
    }
    return buffer;
}

Y_BYTE* sound2(int duration, int hz) {
    Y_BYTE* buffer = (Y_BYTE*)malloc(hz * duration * sizeof(Y_BYTE));

    double f = 0.0;
    for (int t = 0; t < hz * duration; t++) {
        buffer[t] = t * (t >> 4)* sin(f*2*M_PI / (hz * duration));
        f+=1.0 / hz * duration;
    }
    return buffer;
}

Y_BYTE* sound3(int duration, int hz) {
    Y_BYTE* buffer = (Y_BYTE*)malloc(hz * duration * sizeof(Y_BYTE));

    for (int t = 0; t < hz * duration; t++) {
        buffer[t] = t * (t >> 5 | t >> 8) >> (t >> 16);
    }
    return buffer;
}
Y_BYTE* sound4(int duration, int hz) {
    Y_BYTE* buffer = (Y_BYTE*)malloc(hz * duration * sizeof(Y_BYTE));

    for (int t = 0; t < hz * duration; t++) {
        buffer[t] = 430 * (5 * t >> 11 | 5 * t >> 1);
    }
    return buffer;
}
/////////////////////////////////////////////

S_YHWH_SOUND* sound_payload(int duration) {
    S_YHWH_SOUND* v_sound_payload = (S_YHWH_SOUND*)malloc(sizeof(S_YHWH_SOUND) * 4);

    v_sound_payload[0] = (S_YHWH_SOUND){ duration,  8000,  sound1(duration, 8000) };
    v_sound_payload[1] = (S_YHWH_SOUND){ duration,  16000,  sound2(duration, 16000) };
    v_sound_payload[2] = (S_YHWH_SOUND){ duration,  16000,  sound3(duration, 16000) };
    v_sound_payload[3] = (S_YHWH_SOUND){ duration,  8000,  sound4(duration, 8000) };
    
    return v_sound_payload;
}

DWORD WINAPI default_sound_maker(S_YHWH_SOUND s) {
    HWAVEOUT hwo = 0;
    HANDLE hev = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!hev) { free(s.buffer); return 1; }

    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, s.hz, s.hz, 1, 8, 0 };
    MMRESULT mm = waveOutOpen(&hwo, WAVE_MAPPER, &wfx, (DWORD_PTR)hev, 0, CALLBACK_EVENT);
    if (mm != MMSYSERR_NOERROR) { CloseHandle(hev); free(s.buffer); return 2; }

    WAVEHDR wh = { 0 };
    wh.lpData = (LPSTR)s.buffer;
    wh.dwBufferLength = (DWORD)(s.hz * s.duration);

    if ((mm = waveOutPrepareHeader(hwo, &wh, sizeof wh)) != MMSYSERR_NOERROR) {
        waveOutClose(hwo); CloseHandle(hev); free(s.buffer); return 3;
    }
    if ((mm = waveOutWrite(hwo, &wh, sizeof wh)) != MMSYSERR_NOERROR) {
        waveOutUnprepareHeader(hwo, &wh, sizeof wh);
        waveOutClose(hwo); CloseHandle(hev); free(s.buffer); return 4;
    }

    // 재생 완료 대기: 이벤트는 MM_WOM_DONE 때 깜빡이며, 한 버퍼만 쓰니 1번이면 충분
    while (!(wh.dwFlags & WHDR_DONE)) {
        WaitForSingleObject(hev, INFINITE);
    }

    waveOutUnprepareHeader(hwo, &wh, sizeof wh);
    waveOutClose(hwo);
    CloseHandle(hev);
    free(s.buffer);
    return 0;
}
