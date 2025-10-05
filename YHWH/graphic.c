#include "YHWH.h"

extern int screen_width, screen_height;

static inline void rainbow_normalize(S_YHWH_RGB* c) {
    // 이미 휠의 변 위라면 그대로 사용
    int on_edge =
        (c->b == 0 && (c->r == 255 || c->g == 255 || c->r == 0 || c->g == 0)) ||
        (c->r == 0 && (c->g == 255 || c->b == 255 || c->g == 0 || c->b == 0)) ||
        (c->g == 0 && (c->r == 255 || c->b == 255 || c->r == 0 || c->b == 0));
    if (!on_edge) { *c = (S_YHWH_RGB){ 255, 0, 0 }; } // 기본값: 빨강에서 시작
}

S_YHWH_RGB* rainbow_rgb_step(S_YHWH_RGB* c, Y_BYTE step) {
    if (!c) return;
    if (step == 0) step = 1;

    while (step--) {
        if (c->r == 255 && c->b == 0 && c->g < 255) { c->g++; }      // (255,0,0) → (255,255,0)
        else if (c->g == 255 && c->b == 0 && c->r > 0) { c->r--; }      // (255,255,0) → (0,255,0)
        else if (c->r == 0 && c->g == 255 && c->b < 255) { c->b++; }      // (0,255,0) → (0,255,255)
        else if (c->r == 0 && c->b == 255 && c->g > 0) { c->g--; }      // (0,255,255) → (0,0,255)
        else if (c->g == 0 && c->b == 255 && c->r < 255) { c->r++; }      // (0,0,255) → (255,0,255)
        else if (c->r == 255 && c->g == 0 && c->b > 0) { c->b--; }      // (255,0,255) → (255,0,0)
        else { rainbow_normalize(c); } // 변 밖이면 복구
    }
    return c;
}

static int safe_mul_size_t(size_t a, size_t b, size_t* out) {
    if (!out) return 0;
    if (a == 0 || b == 0) { *out = 0; return 1; }
    if (a > SIZE_MAX / b) return 0;
    *out = a * b; return 1;
}
S_YHWH_RGB* graphic1(int toggle_count_num, int toggle_unit, S_YHWH_RGB rgb)
{
    if (screen_width <= 0 || screen_height <= 0) return NULL;

    size_t px_per_frame;
    if (!safe_mul_size_t((size_t)screen_width, (size_t)screen_height, &px_per_frame)) return NULL;

    size_t total_bytes;
    if (!safe_mul_size_t(px_per_frame, sizeof(S_YHWH_RGB), &total_bytes)) return NULL;

    S_YHWH_RGB* buf = (S_YHWH_RGB*)malloc(total_bytes);
    if (!buf) return NULL;

    int toggle_value = 0;
    int toggle_count = toggle_count_num;

    // 한 프레임 생성: 가로 방향 사인 패턴 + N줄마다 토글
    for (int y = 0; y < screen_height; ++y) {
        // 한 행 시작 시 컬럼 진행 비율 0..1
        for (int x = 0; x < screen_width; ++x) {
            size_t i = (size_t)y * (size_t)screen_width + (size_t)x;

            double t = (double)x / (double)screen_width;  // 0.0 ~ 1.0
            // toggle_value에 따라 위상/세기 바꾸기(예시)
            double phase = toggle_value ? 0.0 : 0.5;
            int r = (int)(rgb.r * (0.5 + 0.5 * sin((t + phase) * 2.0 * M_PI)));
            int g = (int)(rgb.g * (0.5 + 0.5 * sin((t + phase) * 2.0 * M_PI)));
            int b = (int)(rgb.b * (0.5 + 0.5 * sin((t + phase) * 2.0 * M_PI)));

            buf[i].r = r;
            buf[i].g = g;
            buf[i].b = b;
        }

        // 행이 끝날 때만 카운트 증가
        toggle_count++;
        if (toggle_count >= toggle_unit) {
            toggle_value = !toggle_value;
            toggle_count = 0;  // 반드시 리셋
        }
    }

    return buf; // 해제는 호출자(default_graphic_maker)에서
}

static inline void fill_square(
    S_YHWH_RGB* buf, int W, int H,
    int x, int y, int d, S_YHWH_RGB c)
{
    if (!buf || W <= 0 || H <= 0 || d <= 0) return;

    // 화면 밖 시작점 정리(음수 허용 시 클램프)
    if (x >= W || y >= H) return;
    if (x < 0) { d += x; x = 0; }
    if (y < 0) { d += y; y = 0; }
    if (d <= 0) return;

    int xend = x + d; if (xend > W) xend = W;
    int yend = y + d; if (yend > H) yend = H;

    for (int yy = y; yy < yend; ++yy) {
        size_t row = (size_t)yy * (size_t)W;
        for (int xx = x; xx < xend; ++xx) {
            buf[row + (size_t)xx] = c;
        }
    }
}

S_YHWH_RGB* graphic2(int d)
{
    if (screen_width <= 0 || screen_height <= 0) return NULL;

    size_t px_per_frame;
    if (!safe_mul_size_t((size_t)screen_width, (size_t)screen_height, &px_per_frame))
        return NULL;

    S_YHWH_RGB* buf = (S_YHWH_RGB*)malloc(px_per_frame * sizeof(S_YHWH_RGB));
    if (!buf) return NULL;

    // 배경 초기화(검정)
    memset(buf, 0, px_per_frame * sizeof(S_YHWH_RGB));
    
    // 10픽셀 간격 타일에 랜덤 색 사각형 채우기
    const int step = d;
    for (int y = 0; y < screen_height; y += step) {
        for (int x = 0; x < screen_width; x += step) {
            
            S_YHWH_RGB c = { rand() % 256,rand() % 256,rand() % 256 };
            fill_square(buf, screen_width, screen_height, x, y, step, c);
        }
    }
    return buf; // 해제는 호출자 책임
}

static inline int _3stride24(int W) { return ((W * 24 + 31) & ~31) >> 3; }

/* 위로 dy_rows만큼 스크롤하고, 잘려나간 윗부분을 아래에 "붙이는" 랩 스크롤 */
static void scroll_up_wrap_stride24(BYTE* buf, int W, int H, int stride, int dy_rows) {
    if (!buf || W <= 0 || H <= 0 || stride < W * 3) return;
    dy_rows %= H;                 // H 이상이면 한 바퀴 돌아 동일
    if (dy_rows <= 0) return;

    const size_t band_bytes = (size_t)stride * dy_rows;
    BYTE* tmp = (BYTE*)malloc(band_bytes);
    if (!tmp) return;             // 메모리 부족 시 그냥 무시

    /* 1) 윗부분 저장 */
    memcpy(tmp, buf, band_bytes);
    /* 2) 본체를 위로 당김 */
    memmove(buf, buf + band_bytes, (size_t)stride * (H - dy_rows));
    /* 3) 저장했던 윗부분을 아래에 붙임 */
    memcpy(buf + (size_t)stride * (H - dy_rows), tmp, band_bytes);

    free(tmp);
}

S_YHWH_RGB* graphic3(HDC* hdc)
{
    const int W = screen_width, H = screen_height;
    if (!hdc || !*hdc || W <= 0 || H <= 0) return NULL;

    // screen_dc에 현재 선택된 비트맵이 있어야 함
    HBITMAP hbmp = (HBITMAP)GetCurrentObject(*hdc, OBJ_BITMAP);
    if (!hbmp) return NULL;

    // 24bpp(top-down) 비트맵 정보 (BGR + stride)
    BITMAPINFO bmi; ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = W;
    bmi.bmiHeader.biHeight = -H;            // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;            // 24bpp (BGR)
    bmi.bmiHeader.biCompression = BI_RGB;

    const int stride = _3stride24(W);
    const size_t bytes_bgr = (size_t)stride * H;

    // 1) 비트맵 → 임시 BGR(stride) 버퍼
    BYTE* bgr = (BYTE*)malloc(bytes_bgr);
    if (!bgr) return NULL;

    if (!GetDIBits(*hdc, hbmp, 0, (UINT)H, bgr, &bmi, DIB_RGB_COLORS)) {
        free(bgr);
        return NULL;
    }

    // 2) 행 단위 스크롤 (반복 재생을 위해 ↓ Step 누적)
    const int dy_rows = 20; // 필요 시 외부에서 조절
    scroll_up_wrap_stride24(bgr, W, H, stride, dy_rows);

    // 3) 스크롤 결과를 같은 비트맵에 되쓰기 (다음 프레임의 소스가 됨)
    if (!SetDIBits(*hdc, hbmp, 0, (UINT)H, bgr, &bmi, DIB_RGB_COLORS)) {
        free(bgr);
        return NULL;
    }

    // 4) 호출자(run_buffer)가 쓰기 좋게 RGB(무패딩)로 변환해서 반환
    S_YHWH_RGB* out = (S_YHWH_RGB*)malloc((size_t)W * (size_t)H * sizeof(S_YHWH_RGB));
    if (!out) { free(bgr); return NULL; }

    for (int y = 0; y < H; ++y) {
        const BYTE* s = bgr + (size_t)y * stride;      // B,G,R,B,G,R,...
        S_YHWH_RGB* d = out + (size_t)y * (size_t)W;   // R,G,B (무패딩)
        for (int x = 0; x < W; ++x) {
            d[x].b = s[x * 3 + 0];
            d[x].g = s[x * 3 + 1];
            d[x].r = s[x * 3 + 2];
        }
    }

    free(bgr);
    return out; // 호출자(run_buffer)가 free
}

static inline int posmod(int a, int m) { int r = a % m; return (r < 0) ? r + m : r; }

/* 안전한 수평선 그리기 (RGB, 무패딩 버퍼 가정) */
static inline void draw_horizontal_line(S_YHWH_RGB* buf, S_YHWH_RGB c, int x, int y, int len) {
    if (!buf || len <= 0) return;
    if ((unsigned)y >= (unsigned)screen_height) return;

    if (x < 0) { len += x; x = 0; }          // 왼쪽 넘치면 길이 줄이기
    if (x >= screen_width || len <= 0) return;

    int xend = x + len;
    if (xend > screen_width) xend = screen_width;

    S_YHWH_RGB* row = buf + (size_t)y * (size_t)screen_width;
    for (int xx = x; xx < xend; ++xx) row[xx] = c;
}
static inline void draw_horizontal_line_clipped(
    S_YHWH_RGB* buf, S_YHWH_RGB color, int x, int y, int len)
{
    if (!buf || len <= 0) return;
    if (y < 0 || y >= screen_height) return;

    int x0 = x;
    int x1 = x + len; // exclusive
    if (x1 <= 0 || x0 >= screen_width) return;

    if (x0 < 0) x0 = 0;
    if (x1 > screen_width) x1 = screen_width;

    S_YHWH_RGB* row = buf + (size_t)y * (size_t)screen_width;
    for (int xi = x0; xi < x1; ++xi) row[xi] = color;
}
S_YHWH_RGB* graphic4(unsigned int dx, int step, double k) {
    if (screen_width <= 0 || screen_height <= 0) return NULL;
    if (step <= 0) step = 1;                             // 0이면 무한루프 방지

    size_t px;
    if (!safe_mul_size_t((size_t)screen_width, (size_t)screen_height, &px)) return NULL;

    S_YHWH_RGB* buf = (S_YHWH_RGB*)malloc(px * sizeof(S_YHWH_RGB));
    if (!buf) return NULL;
    memset(buf, 0, px * sizeof(S_YHWH_RGB));            // 배경: 검정

    S_YHWH_RGB color = (S_YHWH_RGB){ 255, 0, 0 };

    // dx를 step 기준으로만 쓰면 래핑이 자연스러움
    const int dx_mod = (int)(dx % (unsigned)step);

    for (int y = 0; y < screen_height; ++y) {
        const int phase = (int)lrint(step * sin(k * y));
        const int start = posmod(phase - dx_mod, step);

        // 핵심: 왼쪽 경계 밖에서부터 하나 먼저 그려 경계 빈틈 제거
        int x = start - step;
        while (x < screen_width) {
            draw_horizontal_line_clipped(buf, color, x, y, step);
            rainbow_rgb_step(&color, (Y_BYTE)step); // 색 변화
            x += step;
        }
    }

    return buf; // run_buffer가 RGB→BGR로 복사해 줄 것. 해제는 호출자.
}

static inline int stride24(int w) { return (w * 3 + 3) & ~3; }

void run_buffer(S_YHWH_RGB* buffer, BYTE* dst, int st, int W, int H, int fps, HDC hdc, HDC memdc) {
    const S_YHWH_RGB* src = buffer; // 한 프레임만 사용

    // 소스(그래픽1 결과)를 DIB에 복사 (RGB → BGR)
    for (int y = 0; y < H; ++y) {
        BYTE* drow = dst + y * st;
        if (src) {
            const S_YHWH_RGB* srow = src + (size_t)y * (size_t)W;
            for (int x = 0; x < W; ++x) {
                const S_YHWH_RGB p = srow[x];
                drow[x * 3 + 0] = p.b;
                drow[x * 3 + 1] = p.g;
                drow[x * 3 + 2] = p.r;
            }
        }
        else {
            // (버퍼가 NULL일 때 대비) 자홍색
            for (int x = 0; x < W; ++x) {
                drow[x * 3 + 0] = 255;
                drow[x * 3 + 1] = 0;
                drow[x * 3 + 2] = 255;
            }
        }
    }

    BitBlt(hdc, 0, 0, W, H, memdc, 0, 0, SRCCOPY);
    Sleep(1000.0 / fps);
    if (buffer) free(buffer);
}
DWORD WINAPI default_graphic_maker(int v_graphic_duration)
{
    const int fps = FRAME_RATE;
    const int W = screen_width, H = screen_height;
    if (W <= 0 || H <= 0 || v_graphic_duration <= 0) { return 0; } // 잘못된 free 삭제 + 반환값 명시

    // 출력 대상 DC: 콘솔창(테스트용). GUI 윈도우가 있으면 그 HWND로 바꿔.
    HWND hwnd = GetConsoleWindow();
    HDC hdc = hwnd ? GetDC(hwnd) : GetDC(NULL);
    if (!hdc) { return; }

    // 24bpp Top-Down DIB 1개 생성
    BITMAPINFO bi = { 0 };
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = W;
    bi.bmiHeader.biHeight = -H;       // top-down
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 24;       // BGR
    bi.bmiHeader.biCompression = BI_RGB;

    void* bits = NULL;
    HBITMAP bmp = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, &bits, NULL, 0);
    if (!bmp || !bits) {
        if (hwnd) ReleaseDC(hwnd, hdc); else ReleaseDC(NULL, hdc);
        return;
    }

    HDC memdc = CreateCompatibleDC(hdc);
    HBITMAP old = (HBITMAP)SelectObject(memdc, bmp);

    HDC screen_dc = CreateCompatibleDC(hdc);
    HBITMAP snap = CreateCompatibleBitmap(hdc, W, H);
    HBITMAP oldS = (HBITMAP)SelectObject(screen_dc, snap);
    BitBlt(screen_dc, 0, 0, W, H, hdc, 0, 0, SRCCOPY); // 이제 성공

    const int st = stride24(W);
    BYTE* dst = (BYTE*)bits;
    const size_t pxpf = (size_t)W * (size_t)H;
    const int total_frames = v_graphic_duration * fps;
    int toggle_unit = 2;
    int toggle_count_num = 0;
    clock_t time_value = clock();
    S_YHWH_RGB rgb_1 = { 255,0,0 };
    int dx = 1; int step = 1; double t = 0.001;
    int graphic2_d=1;
    for (int f = 0; f < total_frames; ++f) {
        if (clock()-time_value<= v_graphic_duration*1000 * (1.0/4)) {
            rgb_1 = *rainbow_rgb_step(&rgb_1, 5);
            run_buffer(graphic1(toggle_count_num, toggle_unit, rgb_1), dst, st, W, H, fps, hdc, memdc);
            toggle_count_num++;
            if (toggle_count_num == toggle_unit) {
                toggle_count_num = 0;
            }
            if (toggle_count_num == 0) {
                toggle_unit++;
            }
        }
        else if (clock() - time_value <= v_graphic_duration * 1000 * (2.0/4)) {
            run_buffer(graphic2(graphic2_d), dst, st, W, H, fps, hdc, memdc);
            graphic2_d++;
        }
        
        else if (clock() - time_value <= v_graphic_duration * 1000 * (3.0 / 4)) {
            HICON hErrorIcon = LoadIcon(NULL, IDI_ERROR);
            run_buffer(graphic3(&screen_dc), dst, st, W, H, fps, hdc, memdc);
            DrawIconEx(screen_dc, rand()% screen_width, rand() % screen_height, \
                hErrorIcon, ICON_D, ICON_D, 0, NULL, DI_NORMAL | DI_COMPAT | DI_DEFAULTSIZE);
            DestroyIcon(hErrorIcon);
        }
        else if (clock() - time_value <= v_graphic_duration * 1000) {
            run_buffer(graphic4(dx, step, t), dst, st, W, H, fps, hdc, memdc);
            dx += 2; step++; t += 0.001;
        }

    }

    // 정리
    SelectObject(memdc, old);
    DeleteObject(bmp);
    DeleteDC(memdc);
    SelectObject(screen_dc, oldS);
    DeleteObject(snap);
    DeleteDC(screen_dc);
    if (hwnd) ReleaseDC(hwnd, hdc); else ReleaseDC(NULL, hdc);

    return 0;
}