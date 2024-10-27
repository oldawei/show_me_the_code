# corePKCS11
CORE_PKCS11_DIR = $(SOURCE_DIR)/FreeRTOSv202406.01-LTS/corePKCS11

CORE_PKCS11_FLAG = -I$(CORE_PKCS11_DIR)/source/include \
                   -I$(CORE_PKCS11_DIR)/source/portable/os \
                   -I$(CORE_PKCS11_DIR)/source/dependency/3rdparty/mbedtls_utils \
                   -I$(CORE_PKCS11_DIR)/source/dependency/3rdparty/pkcs11/published/2-40-errata-1

CORE_PKCS11_C_SRCS = $(wildcard $(CORE_PKCS11_DIR)/source/*.c)
CORE_PKCS11_C_SRCS += $(wildcard $(CORE_PKCS11_DIR)/source/portable/os/*.c)
CORE_PKCS11_C_SRCS += $(wildcard $(CORE_PKCS11_DIR)/source/portable/mbedtls/*.c)
CORE_PKCS11_C_SRCS += $(wildcard $(CORE_PKCS11_DIR)/source/dependency/3rdparty/mbedtls_utils/*.c)

# corePKCS11-virt-port
CORE_PKCS11_FLAG += -I$(SOURCE_DIR)/qemu-virt-port/corePKCS11
CORE_PKCS11_C_SRCS += $(wildcard $(SOURCE_DIR)/qemu-virt-port/corePKCS11/*.c)

# corePKCS11-demo
CORE_PKCS11_FLAG += -I$(SOURCE_DIR)/corePKCS11-demo
CORE_PKCS11_C_SRCS += $(wildcard $(SOURCE_DIR)/corePKCS11-demo/*.c)

SRCS += $(CORE_PKCS11_C_SRCS)
CFLAGS += $(CORE_PKCS11_FLAG)

