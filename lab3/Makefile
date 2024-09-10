
CROSS_COMPILER_PREFIX = aarch64-linux-gnu
SRCS = $(wildcard ./src/*.c) main.c
OBJS = $(SRCS:.c=.o)

CFLAGS = -Wall -O2 -ffreestanding -nostdlib -nostartfiles

all: clean kernel8.img

start.o: start.S
	$(CROSS_COMPILER_PREFIX)-gcc $(CFLAGS) -c start.S -o start.o

%.o: %.c
	$(CROSS_COMPILER_PREFIX)-gcc $(CFLAGS) -I ./inc -c $< -o $@

kernel8.img: start.o $(OBJS)
	$(CROSS_COMPILER_PREFIX)-ld -nostdlib start.o $(OBJS) -T linker.ld -o kernel8.elf
	$(CROSS_COMPILER_PREFIX)-objcopy -O binary kernel8.elf kernel8.img
	$(CROSS_COMPILER_PREFIX)-size kernel8.elf
	rm kernel8.elf *.o ./src/*.o >/dev/null 2>/dev/null || true


clean:
	rm kernel8.elf kernel8.img *.o ./src/*.o >/dev/null 2>/dev/null || true

run:
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb 
