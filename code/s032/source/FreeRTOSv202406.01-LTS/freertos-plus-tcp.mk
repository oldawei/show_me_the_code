TCP_DIR1 = $(SOURCE_DIR)/FreeRTOSv202406.01-LTS/FreeRTOS-Plus-TCP
TCP_DIR = $(TCP_DIR1)/source

TCP_FLAG = -I$(TCP_DIR)/include \
           -I$(TCP_DIR)/portable/Compiler/GCC \
           -I$(TCP_DIR)/portable/NetworkInterface/virtio-mmio-net \
           -I$(TCP_DIR1)/tools/tcp_utilities/include

TCP_C_SRCS = $(wildcard $(TCP_DIR)/*.c)
TCP_C_SRCS += $(TCP_DIR)/portable/BufferManagement/BufferAllocation_1.c
TCP_C_SRCS += $(TCP_DIR)/portable/NetworkInterface/virtio-mmio-net/NetworkInterface.c

SRCS += $(TCP_C_SRCS)
CFLAGS += $(TCP_FLAG) -DTCPIP=4
