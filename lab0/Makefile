
CROSS_COMPILER_PREFIX = aarch64-linux-gnu
SRCS = $(wildcard lib/*.c)
OBJS = $(SRCS:.c=.o)

CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles

all: clean kernel8.img

start.o: start.S
	$(CROSS_COMPILER_PREFIX)-gcc $(CFLAGS) -c start.S -o start.o

%.o: %.c
	aarch64-linux-gnu-gcc $(CFLAGS) -I ./lib/include -c $< -o $@

kernel8.img: start.o $(OBJS)
	$(CROSS_COMPILER_PREFIX)-ld -nostdlib start.o $(OBJS) -T linker.ld -o kernel8.elf
	$(CROSS_COMPILER_PREFIX)-objcopy -O binary kernel8.elf kernel8.img
	$(CROSS_COMPILER_PREFIX)-size kernel8.elf

clean:
	rm kernel8.elf *.o ./lib/*.o >/dev/null 2>/dev/null || true

run:
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial null -serial stdio
