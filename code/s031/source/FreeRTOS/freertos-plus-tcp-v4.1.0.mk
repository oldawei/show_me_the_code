TCP_DIR = $(SOURCE_DIR)/FreeRTOS/FreeRTOS-Plus-TCPV4.1.0

TCP_FLAG = -I$(TCP_DIR)/include \
			-I$(TCP_DIR)/portable/Compiler/GCC \
			-I$(TCP_DIR)/portable/NetworkInterface/virtio-mmio-net

TCP_C_SRCS = $(wildcard $(TCP_DIR)/*.c)
TCP_C_SRCS += $(TCP_DIR)/portable/BufferManagement/BufferAllocation_1.c
TCP_C_SRCS += $(TCP_DIR)/portable/NetworkInterface/virtio-mmio-net/NetworkInterface.c

SRCS += $(TCP_C_SRCS)
CFLAGS += $(TCP_FLAG) -DTCPIP=4