RTOS_DIR = $(SOURCE_DIR)/FreeRTOS

RTOS_FLAG = -I$(RTOS_DIR)/KernelV10.5.1/include \
			-I$(RTOS_DIR)/KernelV10.5.1/portable/GCC/RISC-V \
			-I$(RTOS_DIR)/KernelV10.5.1/portable/GCC/RISC-V/chip_specific_extensions\RISCV_MTIME_CLINT_no_extensions \
			-I$(RTOS_DIR)/qemu-virt-port

RTOS_C_SRCS = $(wildcard $(RTOS_DIR)/KernelV10.5.1/*.c)
RTOS_C_SRCS += $(RTOS_DIR)/KernelV10.5.1/portable/GCC/RISC-V/port.c
RTOS_C_SRCS += $(RTOS_DIR)/KernelV10.5.1/portable/MemMang/heap_4.c
RTOS_C_SRCS += $(wildcard $(RTOS_DIR)/qemu-virt-port/*.c)

RTOS_S_SRCS = $(RTOS_DIR)/KernelV10.5.1/portable/GCC/RISC-V/portASM.S \
              $(RTOS_DIR)/qemu-virt-port/vector.S \
			  $(RTOS_DIR)/qemu-virt-port/isr.S

SRCS += $(RTOS_C_SRCS)
ASMS += $(RTOS_S_SRCS)
CFLAGS += $(RTOS_FLAG)