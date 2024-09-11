BUILD_DIR = ./build/locals/
OUTPUT_DIR = $(BUILD_DIR)../
SRC_DIR = ./src/
INC_DIR = ./inc/

SRCS = $(wildcard *.c) $(wildcard $(SRC_DIR)*.c)
SRCS_ASM = $(wildcard *.S)

OBJS = $(addprefix $(BUILD_DIR), $(SRCS:.c=.o))
OBJS_ASM = $(addprefix $(BUILD_DIR), $(SRCS_ASM:.S=.o))

INCLUDES = -I $(INC_DIR)

CCFLAGS = -Wall -O2 -ffreestanding -nostdlib -nostartfiles

ASMFLAGS = $(CCFLAGS)
LDFLAGS = -T linker.ld -nostartfiles -ffreestanding -nostdlib -static


TOOL_CHAIN_PREFIX = aarch64-linux-gnu
CC = $(TOOL_CHAIN_PREFIX)-gcc
LD = $(TOOL_CHAIN_PREFIX)-gcc
SIZE = $(TOOL_CHAIN_PREFIX)-size
OBJCOPY = $(TOOL_CHAIN_PREFIX)-objcopy

all: build_dir kernel8.img
build: all
c: clean

build_dir:
	@mkdir -p $(dir $(OBJS))
	@mkdir -p $(dir $(OBJS_ASM))

$(BUILD_DIR)%.o: %.S
	$(CC) $(CCFLAGS) -c $< -o $@

$(BUILD_DIR)%.o: %.c
	$(CC) $(CCFLAGS) -I $(INC_DIR) -c $< -o $@

kernel8.img: $(OBJS_ASM) $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS_ASM) $(OBJS) -o $(OUTPUT_DIR)/kernel8.elf
	$(OBJCOPY) -O binary $(OUTPUT_DIR)/kernel8.elf $(OUTPUT_DIR)/kernel8.img
	$(SIZE) $(OUTPUT_DIR)/kernel8.elf

clean:
	rm -rf build/

run:
	qemu-system-aarch64 -M raspi3b -kernel $(OUTPUT_DIR)kernel8.img -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb 

debug:
	qemu-system-aarch64 -M raspi3b -kernel $(OUTPUT_DIR)kernel8.img -serial null -serial stdio -s -S -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb 	

load:
	source ~/py-env/bin/activate && python3 send_kernel.py

screen:
	screen /dev/tty.usbserial-0001 115200