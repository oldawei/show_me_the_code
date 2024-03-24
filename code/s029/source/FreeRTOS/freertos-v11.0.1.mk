RTOS_DIR = $(SOURCE_DIR)/FreeRTOS

RTOS_FLAG = -I$(RTOS_DIR)/KernelV11.0.1/include \
			-I$(RTOS_DIR)/KernelV11.0.1/portable/GCC/RISC-V \
			-I$(RTOS_DIR)/KernelV11.0.1/portable/GCC/RISC-V/chip_specific_extensions\RISCV_MTIME_CLINT_no_extensions \
			-I$(RTOS_DIR)/FreeRTOS-Plus-CLI-V1.0.4 \
			-I$(RTOS_DIR)/qemu-virt-port \
			-I$(RTOS_DIR)/FreeRTOS-Plus-FAT-V2.3.3/portable/virtio-blk \
			-I$(RTOS_DIR)/FreeRTOS-Plus-FAT-V2.3.3/include

RTOS_C_SRCS = $(wildcard $(RTOS_DIR)/KernelV11.0.1/*.c)
RTOS_C_SRCS += $(RTOS_DIR)/KernelV11.0.1/portable/GCC/RISC-V/port.c
RTOS_C_SRCS += $(RTOS_DIR)/KernelV11.0.1/portable/MemMang/heap_4.c
RTOS_C_SRCS += $(wildcard $(RTOS_DIR)/qemu-virt-port/*.c)
RTOS_C_SRCS += $(wildcard $(RTOS_DIR)/FreeRTOS-Plus-CLI-V1.0.4/*.c)
RTOS_C_SRCS += $(wildcard $(RTOS_DIR)/FreeRTOS-Plus-FAT-V2.3.3/*.c)
RTOS_C_SRCS += $(wildcard $(RTOS_DIR)/FreeRTOS-Plus-FAT-V2.3.3/portable/virtio-blk/*.c)

RTOS_S_SRCS = $(RTOS_DIR)/KernelV11.0.1/portable/GCC/RISC-V/portASM.S \
              $(RTOS_DIR)/qemu-virt-port/vector.S \
			  $(RTOS_DIR)/qemu-virt-port/isr.S

SRCS += $(RTOS_C_SRCS)
ASMS += $(RTOS_S_SRCS)
CFLAGS += $(RTOS_FLAG)