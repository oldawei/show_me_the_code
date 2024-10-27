#include "common.h"
#include "mbedtls/threading.h"

static void mutex_alt_init(mbedtls_threading_mutex_t *mutex)
{
    if (mutex == NULL) {
        return;
    }

    mutex->mutex = xSemaphoreCreateMutex();
}

static void mutex_alt_free(mbedtls_threading_mutex_t *mutex)
{
    if (mutex == NULL) {
        return;
    }

    vSemaphoreDelete( mutex->mutex );
}

static int mutex_alt_lock(mbedtls_threading_mutex_t *mutex)
{
    if (mutex == NULL) {
        return MBEDTLS_ERR_THREADING_BAD_INPUT_DATA;
    }

    xSemaphoreTake( mutex->mutex, portMAX_DELAY );
    return 0;
}

static int mutex_alt_unlock(mbedtls_threading_mutex_t *mutex)
{
    if (mutex == NULL) {
        return MBEDTLS_ERR_THREADING_BAD_INPUT_DATA;
    }

    xSemaphoreGive( mutex->mutex );
    return 0;
}

void mbedtls_platform_threading_init(void)
{
	mbedtls_threading_set_alt(mutex_alt_init, mutex_alt_free, mutex_alt_lock, mutex_alt_unlock);
}
