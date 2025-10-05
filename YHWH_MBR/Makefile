NASM   := nasm
QEMU   := qemu-system-i386
IMG    := disk.img
STAGE1 := stage1_boot.asm
STAGE2 := stage2_kernel.asm
B1     := stage1.bin
B2     := stage2.bin

AUDIO  ?= -audiodev pa,id=snd0 -machine pcspk-audiodev=snd0

.PHONY: all build clean run debug

all: build

build: $(IMG)

$(B1): $(STAGE1)
	$(NASM) -f bin $(STAGE1) -o $(B1)

$(B2): $(STAGE2)
	$(NASM) -f bin $(STAGE2) -o $(B2)

$(IMG): $(B1) $(B2)
	@echo "[*] Create image"
	dd if=/dev/zero of=$(IMG) bs=1M count=2 status=none
	@echo "[*] Write boot sector"
	dd if=$(B1) of=$(IMG) conv=notrunc status=none
	@echo "[*] Write stage2 at LBA 1.."
	dd if=$(B2) of=$(IMG) bs=512 seek=1 conv=notrunc status=none
	@echo "[*] Done."

run: $(IMG)
	$(QEMU) $(AUDIO) -hda $(IMG) 
clean:
	rm -f $(B1) $(B2) $(IMG)

# 화면/비디오만 빠르게 확인하고 싶을 때, stage2_kernel.asm에
# 위 '전체 흰색 칠하기' 진단 코드 잠깐 넣고 호출
debug: run

