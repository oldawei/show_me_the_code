# tools
PREFIX 		= riscv-none-elf-
CC 			= $(PREFIX)gcc
CPP			= $(PREFIX)cpp
OBJDUMP 	= $(PREFIX)objdump
OBJCOPY 	= $(PREFIX)objcopy
AR 			= $(PREFIX)ar
AS			= $(PREFIX)as
SZ 			= $(PREFIX)size
FE 			= $(PREFIX)readelf
GDB			= $(PREFIX)gdb
LD			= $(PREFIX)ld

ifeq ($(OS),Windows_NT)
DD			= busybox dd
else
DD			= dd
endif

# 	32b only
ABI = -mabi=ilp32 -march=rv32imac_zicsr_zifencei

BUILD_DIR  	= build
SOURCE_DIR	= source
BIN_NAME	= $(BUILD_DIR)/demo

all: DIR $(BIN_NAME).elf

CFLAGS = ${ABI} -Wall -O2
CFLAGS += -Werror -Wfatal-errors
CFLAGS += -funroll-all-loops -fno-common
CFLAGS += -std=c99 -fno-builtin -fno-strict-aliasing
CFLAGS += -fmessage-length=0 -fstack-usage -Wstack-usage=2048
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += --static -nostdlib -nodefaultlibs
#CFLAGS += -g3 -gdwarf-2
CFLAGS += -fno-builtin-printf -DPRINTF_INCLUDE_CONFIG_H=1 -DD_CLOCK_RATE=10000000
CFLAGS += -Iinclude -Isource/cpu -Isource/driver -Isource/library -Isource/platform
CFLAGS += -Isource/driver/virtio

LDFLAGS = -nostartfiles -ffunction-sections -fdata-sections -Tqemu.lds ${ABI} \
	-Wl,-Map,${BIN_NAME}.map -Wl,--gc-sections

SRCS = 	$(SOURCE_DIR)/driver/ns16550.c \
		$(SOURCE_DIR)/driver/cfiflash.c \
		$(SOURCE_DIR)/driver/rtc-goldfish.c \
		$(SOURCE_DIR)/driver/virtio/virtio-ring.c \
		$(SOURCE_DIR)/cpu/riscv32.c \
		$(SOURCE_DIR)/platform/plic.c \
		$(SOURCE_DIR)/platform/clint.c \
		$(SOURCE_DIR)/platform/riscv-virt.c \
		$(SOURCE_DIR)/library/printf/printf.c \
		$(SOURCE_DIR)/main.c

ASMS = $(SOURCE_DIR)/cpu/start.S

OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o) $(ASMS:%.S=$(BUILD_DIR)/%.o)
DEPS = $(SRCS:%.c=$(BUILD_DIR)/%.d) $(ASMS:%.S=$(BUILD_DIR)/%.d)
-include $(DEPS)

$(BUILD_DIR)/flash.img:
	${DD} if=/dev/zero of=$@ bs=32M count=1

qemu: ${BIN_NAME}.elf $(BUILD_DIR)/flash.img
	qemu-system-riscv32 -nographic -machine virt -net none \
  		-chardev stdio,id=con,mux=on -serial chardev:con -smp 1 \
  		-mon chardev=con,mode=readline -bios ${BIN_NAME}.bin \
  		-drive if=pflash,file=$(BUILD_DIR)/flash.img,format=raw,index=1

${BIN_NAME}.elf: $(OBJS) qemu.lds Makefile
	@echo link $@
	@$(CC) $(LDFLAGS) $(OBJS) -o $@

	@echo generate ${BIN_NAME}.bin
	@$(OBJCOPY) -O binary -S $@ ${BIN_NAME}.bin

	@echo generate ${BIN_NAME}.asm
	@$(OBJDUMP) -S -dt $@ > ${BIN_NAME}.asm

	@echo 'File Size:'
	$(SZ) $@

	@echo 'File Headers:'
	$(FE) -l $@

$(BUILD_DIR)/%.o: %.c Makefile
	@mkdir -p $(@D)
	@echo building $<
	@$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/%.o: %.S Makefile
	@mkdir -p $(@D)
	@echo building $<
	@$(CC) $(CFLAGS) -MMD -MP -c $< -o $@


DIR:
	@mkdir -p ${BUILD_DIR} > /dev/null 2>&1

clean:
	rm -rf $(BUILD_DIR)
