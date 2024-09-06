BUILD_DIR = build
SRCDIR = lib
CROSS_COMPILER_PREFIX = aarch64-linux-gnu
SRC_FILE = $(wildcard lib/*.c) main.c
OBJ_FILE = ${subst /$(SRCDIR),, $(addprefix $(BUILD_DIR)/, $(SRC_FILE:.c=.o))}  # $(SRCS:.c=.o)
CFLAGS = -Wall -ffreestanding -nostdlib -nostartfiles -ggdb# -nostdinc

all: clean kernel8.img

$(BUILD_DIR)/start.o: start.S
	$(CROSS_COMPILER_PREFIX)-gcc $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/main.o: main.c
	$(CROSS_COMPILER_PREFIX)-gcc $(CFLAGS) -I ./lib/include -c $< -o $@

$(BUILD_DIR)/%.o: $(SRCDIR)/%.c
	$(CROSS_COMPILER_PREFIX)-gcc $(CFLAGS) -I ./lib/include -c $< -o $@

kernel8.img: $(BUILD_DIR)/start.o $(OBJ_FILE)
	$(CROSS_COMPILER_PREFIX)-ld -nostdlib $(BUILD_DIR)/start.o $(OBJ_FILE) -T link.ld -o kernel8.elf
	$(CROSS_COMPILER_PREFIX)-objcopy -O binary kernel8.elf kernel8.img
	$(CROSS_COMPILER_PREFIX)-size kernel8.elf

clean:
	rm kernel8.elf ./build/*.o ./lib/*.o >/dev/null 2>/dev/null || true

run:
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb 

debug:
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial null -serial stdio -s -S -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb 	

load:
	source ~/py-env/bin/activate && python3 send_kernel.py

screen:
	screen /dev/tty.usbserial-0001 115200