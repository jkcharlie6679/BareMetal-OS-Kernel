
CROSS_COMPILER_PREFIX = aarch64-linux-gnu
SRCS = $(wildcard src/*.c) main.c
OBJS = $(SRCS:.c=.o)

CFLAGS = -Wall -O2 -ffreestanding -nostdlib -nostartfiles # -nostdinc

all: clean bootloader.img

start.o: start.S
	$(CROSS_COMPILER_PREFIX)-gcc $(CFLAGS) -c start.S -o start.o

%.o: %.c
	$(CROSS_COMPILER_PREFIX)-gcc $(CFLAGS) -I ./inc -c $< -o $@

bootloader.img: start.o $(OBJS)
	$(CROSS_COMPILER_PREFIX)-ld -nostdlib start.o $(OBJS) -T linker.ld -o kernel8.elf
	$(CROSS_COMPILER_PREFIX)-objcopy -O binary kernel8.elf bootloader.img
	$(CROSS_COMPILER_PREFIX)-size kernel8.elf

clean:
	rm bootloader.elf bootloader.img *.o ./src/*.o >/dev/null 2>/dev/null || true

bootloader:
	qemu-system-aarch64 -M raspi3b -kernel bootloader.img -serial null -serial pty -dtb ./../bcm2710-rpi-3-b-plus.dtb -initrd ./../initramfs.cpio # pty for the pseudo TTY

run:
	qemu-system-aarch64 -M raspi3b -kernel bootloader.img -serial null -serial stdio -dtb ./../bcm2710-rpi-3-b-plus.dtb -initrd initramfs.cpio
