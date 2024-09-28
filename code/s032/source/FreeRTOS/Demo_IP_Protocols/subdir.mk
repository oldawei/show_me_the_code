IPSVR_DIR = $(SOURCE_DIR)/FreeRTOS/Demo_IP_Protocols

IPSVR_FLAG = -I$(IPSVR_DIR)/include

IPSVR_C_SRCS = $(wildcard $(IPSVR_DIR)/Common/*.c)
IPSVR_C_SRCS += $(wildcard $(IPSVR_DIR)/FTP/*.c)
IPSVR_C_SRCS += $(wildcard $(IPSVR_DIR)/HTTP_Server/*.c)
#IPSVR_C_SRCS += $(wildcard $(IPSVR_DIR)/NTP/*.c)

SRCS += $(IPSVR_C_SRCS)
CFLAGS += $(IPSVR_FLAG)