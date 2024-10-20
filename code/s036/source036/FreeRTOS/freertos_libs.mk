RTOS_LIBS_DIR = $(SOURCE_DIR)/FreeRTOS

RTOS_LIBS_FLAG = -I$(RTOS_LIBS_DIR)/FreeRTOS-Plus-CLI-V1.0.4 \
		-I$(RTOS_LIBS_DIR)/FreeRTOS-Plus-FAT-V2.3.3/portable/virtio-blk \
		-I$(RTOS_LIBS_DIR)/FreeRTOS-Plus-FAT-V2.3.3/include \
		-I$(RTOS_LIBS_DIR)/qemu-virt-port

RTOS_LIBS_C_SRCS += $(wildcard $(RTOS_LIBS_DIR)/qemu-virt-port/*.c)
RTOS_LIBS_C_SRCS += $(wildcard $(RTOS_LIBS_DIR)/FreeRTOS-Plus-CLI-V1.0.4/*.c)
RTOS_LIBS_C_SRCS += $(wildcard $(RTOS_LIBS_DIR)/FreeRTOS-Plus-FAT-V2.3.3/*.c)
RTOS_LIBS_C_SRCS += $(wildcard $(RTOS_LIBS_DIR)/FreeRTOS-Plus-FAT-V2.3.3/portable/virtio-blk/*.c)

RTOS_LIBS_S_SRCS = $(RTOS_LIBS_DIR)/qemu-virt-port/vector.S \
                   $(RTOS_LIBS_DIR)/qemu-virt-port/isr.S

SRCS += $(RTOS_LIBS_C_SRCS)
ASMS += $(RTOS_LIBS_S_SRCS)
CFLAGS += $(RTOS_LIBS_FLAG)
