#include "common.h"

#if defined(MBEDTLS_ENTROPY_C)

#include "mbedtls/entropy.h"
#include "entropy_poll.h"
#include "mbedtls/error.h"
#include "printf/printf.h"

u64 goldfish_rtc_read_time_usec(void);
void xoshiro128starstar_init(uint32_t seed[4]);
void xoshiro128starstar_long_jump(void);
uint32_t xoshiro128starstar_next(void);

static int hw_init_done = 0;

/**
 * \brief           Entropy poll callback for a hardware source
 *
 * \warning         This is not provided by Mbed TLS!
 *                  See \c MBEDTLS_ENTROPY_HARDWARE_ALT in mbedtls_config.h.
 *
 * \note            This must accept NULL as its first argument.
 */
int mbedtls_hardware_poll(void *data,
                          unsigned char *output, size_t len, size_t *olen)
{
    if (hw_init_done == 0)
    {
        hw_init_done = 1;
        uint32_t seed[4];
        u64 secs = goldfish_rtc_read_time_usec();

        seed[0] = (uint32_t)(secs & 0xFFFFFFFF);
        seed[1] = (uint32_t)(secs >> 32);
        seed[2] = 0x12345678;
        seed[3] = 0x9abcdef0;
        printf("seed: %08x %08x %08x %08x", seed[0], seed[1], seed[2], seed[3]);
        xoshiro128starstar_init(seed);
        xoshiro128starstar_long_jump();
    }

    ((void) data);
    *olen = 0;

    while (len != 0) {
        if (len >= 4) {
            uint32_t *pt = (uint32_t *)output;
            *pt = xoshiro128starstar_next();
            output += sizeof(uint32_t);
            *olen += sizeof(uint32_t);
            len -= sizeof(uint32_t);
        } else {
            u32 v = xoshiro128starstar_next();
            memcpy(output, &v, len);
            *olen += len;
            len = 0;
        }
    }

    return 0;
}

#endif