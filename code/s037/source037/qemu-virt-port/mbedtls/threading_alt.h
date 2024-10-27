#ifndef THREADING_ALT_H
#define THREADING_ALT_H

#include "FreeRTOS.h"
/* Synchronization primitive include. */
#include "semphr.h"

typedef struct mbedtls_threading_mutex_t {
    SemaphoreHandle_t mutex;
} mbedtls_threading_mutex_t;

void mbedtls_platform_threading_init(void);

#endif // THREADING_ALT_H
