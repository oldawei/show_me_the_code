BACK_OFF_DIR = $(SOURCE_DIR)/FreeRTOSv202406.01-LTS/backoffAlgorithm

BACK_OFF_FLAG = -I$(BACK_OFF_DIR)/source/include

BACK_OFF_C_SRCS = $(wildcard $(BACK_OFF_DIR)/source/*.c)

SRCS += $(BACK_OFF_C_SRCS)
CFLAGS += $(BACK_OFF_FLAG)
