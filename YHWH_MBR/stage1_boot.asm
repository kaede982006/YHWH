; ----- Stage-1 Bootsector (CHS로 20섹터 로드) -----
BITS 16
ORG 0x7C00

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov [BootDrive], dl           ; BIOS가 넘겨준 부팅 드라이브(0x80=첫 HDD)

; 로드 대상 세그먼트:오프셋 = 0000:8000
    mov ax, 0x0000
    mov es, ax
    mov bx, 0x8000

; CHS: 트랙0, 헤드0, 섹터2부터 N=20섹터 로드
    mov dh, 0          ; head
    mov ch, 0          ; track
    mov cl, 2          ; sector (1-based)
    mov ah, 2          ; INT 13h read sectors
    mov al, 20         ; count
    mov dl, [BootDrive]
    int 0x13
    jc disk_error

    jmp 0x0000:0x8000  ; Stage-2 진입

disk_error:
    ; 간단 에러 루프
    mov si, err
.print:
    lodsb
    or al, al
    jz $
    mov ah, 0x0E
    int 0x10
    jmp .print

err db 'Disk read error',0

BootDrive db 0

times 510-($-$$) db 0
dw 0xAA55

