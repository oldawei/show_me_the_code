RTOS_DIR = $(SOURCE_DIR)/FreeRTOSv202406.01-LTS

RTOS_FLAG = -I$(RTOS_DIR)/FreeRTOS-Kernel/include \
            -I$(RTOS_DIR)/FreeRTOS-Kernel/portable/GCC/RISC-V \
            -I$(RTOS_DIR)/FreeRTOS-Kernel/portable/GCC/RISC-V/chip_specific_extensions/RISCV_MTIME_CLINT_no_extensions

RTOS_C_SRCS = $(wildcard $(RTOS_DIR)/FreeRTOS-Kernel/*.c)
RTOS_C_SRCS += $(RTOS_DIR)/FreeRTOS-Kernel/portable/GCC/RISC-V/port.c
RTOS_C_SRCS += $(RTOS_DIR)/FreeRTOS-Kernel/portable/MemMang/heap_4.c

RTOS_S_SRCS = $(RTOS_DIR)/FreeRTOS-Kernel/portable/GCC/RISC-V/portASM.S

SRCS += $(RTOS_C_SRCS)
ASMS += $(RTOS_S_SRCS)
CFLAGS += $(RTOS_FLAG)
