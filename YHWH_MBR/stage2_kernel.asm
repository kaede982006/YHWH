BITS 16
ORG 0x8000

%define VGA_SEG   0xA000
%define BUF_SEG   0x7000

%define SCR_W     320
%define SCR_H     200

%define PIT_CTRL  0x43
%define PIT_CH2   0x42
%define PPI_61    0x61
%define PIT_FREQ  1193182

; --- ticks는 상수로, divisor는 함수형 매크로로 ---
%define TICKS_Q  12
%define TICKS_E   6
%define TICKS_H  24
%define TICKS_DQ 18
%define HZDIV(x) (PIT_FREQ/(x))

start2:
    cli
    xor ax,ax
    mov ds,ax
    mov es,ax
    mov ss,ax          ; ★ 스택 세그먼트 명시
    mov sp,0x9FF0      ; ★ 넉넉한 스택 설정(하위 메모리)
    sti

    mov ax,0x0013
    int 0x10
    cld

    ; --- 오프스크린 버퍼 0으로 초기화(선택) ---
    push es
    mov ax, BUF_SEG
    mov es, ax
    xor di, di
    xor ax, ax
    mov cx, 320*200
    rep stosb
    pop es

    ; 커서 숨김
    mov ah,0x01
    mov ch,0x20
    xor cl,cl
    int 0x10

    ; 초기 값
    mov word [cx_pos], 160
    mov word [cy_pos], 100
    mov word [dx_step], 1
    mov word [dy_step], 1

    ; 애니메이션 틱 디바이더: 2이면 약 110ms마다 1 step
    mov byte [anim_div], 20
	
    mov al, [anim_div]
    mov [anim_acc], al
main_loop:
    ; ===== 멜로디 1주기 재생 =====
    mov si, melody
.note_loop:
    lodsw                    ; AX = divisor (0=rest, FFFF=end)
    cmp ax, 0FFFFh
    je .restart
    mov bx, ax               ; BX=divisor
    lodsw                    ; AX = duration ticks
    mov di, ax               ; DI=duration

    cmp bx, 0
    jne .play_tone
    call spk_off
    jmp short .durate
.play_tone:
    mov ax, bx
    call spk_on_divisor

.durate:
    ; duration 동안 애니메이션을 일정 속도로 진행
    mov ax, di
    call wait_ticks_with_anim

    jmp .note_loop

.restart:
    ; 무한 반복
    jmp main_loop

; ===== PC Speaker =====
; AX=divisor (1..65535), 0이면 호출하지 말 것
spk_on_divisor:
    push dx
    mov al,0xB6             ; ch2, lobyte/hibyte, mode3 square
    out PIT_CTRL,al
    mov dx,PIT_CH2
    mov al,al               ; 유지용
    mov al,al               ; no-op
    mov al,al
    ; write AX low/high
    mov al,al
    mov al,al
    mov al,al
    ; 실제 기록
    mov al,al
    ; (위 no-op들은 일부 구식 에뮬에서 타이밍 여유—없어도 무방)
    mov al,al
    mov al,al
    ; ---- 실제로는 아래 두 줄만 필수 ----
    mov al,al
    ; -------------------------------
    mov al,al

    ; 로우/하이 기록
    mov al,al               ; AX 이미 세팅됨
    out dx,al               ; low
    mov al,ah
    out dx,al               ; high

    in  al,PPI_61
    or  al,00000011b        ; gate+data enable
    out PPI_61,al
    pop dx
    ret

spk_off:
    in  al,PPI_61
    and al,11111100b
    out PPI_61,al
    ret
wait_ticks_with_anim:
    push ax
    push bx
    push ds
    mov bx, ax                  ; 남은 ticks (인자)

.next_tick:
    ; 목표 tick = now + 1
    push ds
    mov ax, 0x40
    mov ds, ax
    mov dx, [0x6C]              ; BIOS tick (저워드면 1tick 비교 OK)
    pop ds
    inc dx

    mov word [fs_cnt], 30000    ; ★ 실패-안전 카운터를 메모리에

.wait1:
    call anim_maybe_step

    ; tick 도달?
    push ds
    mov ax, 0x40
    mov ds, ax
    mov ax, [0x6C]
    pop ds
    cmp ax, dx
    jae .tick_ok

    ; ★ loop 금지: CX 의존 제거
    dec word [fs_cnt]
    jnz .wait1

.tick_ok:
    dec bx
    jnz .next_tick

    pop ds
    pop bx
    pop ax
    ret

; 십자가 애니메이션 1스텝: 분주(acc) 기반
; acc==0 일 때만 위치 갱신, 그 외에는 acc만 감소시키고 draw만 수행
anim_maybe_step:
    push cx
    push ax
    push si
    push di

    ; ====== 분주기 처리 ======
    mov al, [anim_acc]
    or  al, al
    jnz .dec_and_draw_only      ; acc != 0 → 1 줄이고 draw만

    ; acc == 0 → 이번 프레임은 '업데이트' 수행 후 acc 재장전
    mov al, [anim_div]
    mov [anim_acc], al

    ; ====== 위치 업데이트(바운스) ======
    ; cx_pos, cy_pos, dx_step, dy_step, SCR_W, SCR_H 는 기존 그대로 사용
    push bx
    push dx
        mov ax, [cx_pos]
        add ax, [dx_step]
        mov bx, SCR_W-10
        cmp ax, bx
        jbe .x_ok
        mov ax, SCR_W-10
        neg word [dx_step]
    .x_ok:
        cmp ax, 10
        jae .x_store
        mov ax, 10
        neg word [dx_step]
    .x_store:
        mov [cx_pos], ax

        mov ax, [cy_pos]
        add ax, [dy_step]
        mov bx, SCR_H-15
        cmp ax, bx
        jbe .y_ok
        mov ax, SCR_H-15
        neg word [dy_step]
    .y_ok:
        cmp ax, 15
        jae .y_store
        mov ax, 15
        neg word [dy_step]
    .y_store:
        mov [cy_pos], ax
    pop dx
    pop bx
    jmp .draw_phase

.dec_and_draw_only:
    dec al
    mov [anim_acc], al          ; acc만 줄이고(아직 0 아님) 이번엔 업데이트 생략

.draw_phase:
    ; ====== 1) 오프스크린 버퍼 클리어 ======
    push es
        mov ax, BUF_SEG
        mov es, ax
        xor di, di
        xor ax, ax
        mov cx, 320*200
        cld
        rep stosb

    ; ====== 2) 십자가 그리기 ======
        mov al, 15
        ; draw_cross_thick가 BP 쓰면 내부에서 push bp/pop bp 하게 하거나
        ; 여기서도 보존해도 됨(권장): push bp / call / pop bp
        call draw_cross_thick

    ; ====== 3) 버퍼→VGA 블릿 ======
    push ds
        mov ax, BUF_SEG
        mov ds, ax
        xor si, si
        mov ax, VGA_SEG
        mov es, ax
        xor di, di
        mov cx, 320*200
        cld
        rep movsb
    pop ds
    pop es

    pop di
    pop si
    pop ax
    pop cx
    ret


draw_cross_thick:
    push ax
    push bx
    push cx
    push dx
    push di
    push si

    mov bx,[cx_pos]     ; center x
    mov dx,[cy_pos]     ; center y

    ; ---- 수평 막대 (두께 THICK) ----
    ; y = dy -1 .. dy +1, x = (bx - 12) .. (bx + 12)
    mov si, dx
    sub si, 1           ; y start
.hrow:
    mov cx, bx
    sub cx, 12
.hrow_span:
    mov di, si          ; y
    call pset_yx
    inc cx
    mov bp, bx
    add bp, 12
    cmp cx, bp
    jle .hrow_span
    inc si
    mov bp, dx
    add bp, 1
    cmp si, bp
    jle .hrow

    ; ---- 수직 막대 (아래쪽 더 길게, 두께 THICK) ----
    ; x = bx-1 .. bx+1, y=(dy-10)..(dy+18)
    mov si, bx
    sub si, 1           ; x start
.vcol:
    mov cx, dx
    sub cx, 10          ; 위쪽 10
.vcol_span:
    mov di, cx          ; y
    mov si, si          ; x 유지
    call pset_xy
    inc cx
    mov bp, dx
    add bp, 18          ; 아래쪽 18 (더 길게)
    cmp cx, bp
    jle .vcol_span
    inc si
    mov bp, bx
    add bp, 1
    cmp si, bp
    jle .vcol

    pop si
    pop di
    pop dx
    pop cx
    pop bx
    pop ax
    ret

; --- 픽셀 헬퍼 (ES=VGA_SEG, DF=0 가정) ---
; pset_yx: (x in CX), (y in DI), AL=color
pset_yx:
    push ax
    push bx
    push dx
    push di
    mov bx, di
    mov dx, bx
    shl bx, 6
    shl dx, 8
    add bx, dx          ; y*320
    add bx, cx          ; + x
    mov di, bx
    stosb
    pop di
    pop dx
    pop bx
    pop ax
    ret

; pset_xy: (x in SI), (y in DI), AL=color
pset_xy:
    push ax
    push bx
    push dx
    push di
    mov bx, di
    mov dx, bx
    shl bx, 6
    shl dx, 8
    add bx, dx
    add bx, si
    mov di, bx
    stosb
    pop di
    pop dx
    pop bx
    pop ax
    ret

; ===== 데이터 =====
cx_pos    dw 160
cy_pos    dw 100
dx_step   dw 1
dy_step   dw 1
anim_div  db 2    ; 애니메이션 분주(작을수록 빠름)
anim_acc  db 2
fs_cnt   dw 0

; ♪=90 기준 양자화: 쿼터=12, 에잇=6, 하프=24 ...
; 아래 멜로디는 44번 ‘Hallelujah’의 대표적 후렴 구절을 쉬움 반주판 기준으로 발췌·반복 구성.
; (음높이는 D장조 근간, 쉼은 divisor=0)
melody:
    ; "할-렐-루-야" (D4 A4 B4 A4 | G4 F#4 G4 -)
    dw HZDIV(294), TICKS_Q
    dw HZDIV(440), TICKS_Q
    dw HZDIV(494), TICKS_Q
    dw HZDIV(440), TICKS_Q
    dw HZDIV(392), TICKS_E
    dw HZDIV(370), TICKS_E
    dw HZDIV(392), TICKS_H
    dw 0,        TICKS_Q

    ; "전능하신 주가 통치하신다" (대표 패턴, 단순화 반복)
    dw HZDIV(587), TICKS_E      ; D5
    dw HZDIV(659), TICKS_E      ; E5
    dw HZDIV(587), TICKS_E
    dw HZDIV(523), TICKS_E
    dw HZDIV(494), TICKS_E
    dw HZDIV(440), TICKS_E
    dw 0,        TICKS_E
    dw HZDIV(587), TICKS_DQ     ; D5 길게
    dw HZDIV(392), TICKS_E
    dw HZDIV(440), TICKS_Q
    dw HZDIV(494), TICKS_Q
    dw 0,        TICKS_E

    ; 반복을 위한 구분(원한다면 더 많은 마디를 같은 규칙으로 이어붙이면 됨)
    dw 0FFFFh, 0             ; 끝표시 → 메인 루프에서 처음으로 점프(무한반복)

; ===== 종료 없음 (무한 루프) =====

