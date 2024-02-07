#ifndef __CFIFLASH_H__
#define __CFIFLASH_H__

#include "types.h"

#define CFIFLASH_MAX_NUM            2
#define CFIFLASH_CAPACITY           (32 * 1024 * 1024)
#define CFIFLASH_ERASEBLK_SIZE      (128 * 1024 * 2)    /* fit QEMU of 2 banks  */
#define CFIFLASH_ERASEBLK_WORDS     (CFIFLASH_ERASEBLK_SIZE / sizeof(u32))
#define CFIFLASH_ERASEBLK_WORDMASK  (~(CFIFLASH_ERASEBLK_WORDS - 1))

/* Results of Flash Functions */
typedef enum {
    FLASH_OK = 0,     /* 0: Successful */
    FLASH_ERROR       /* 1: R/W Error */
} FLASH_DRESULT;

unsigned CfiFlashSec2Bytes(unsigned sector);

int CfiFlashInit(u32 pdrv, u32 priv);
int CfiFlashQuery(u32 pdrv);
int CfiFlashRead(u32 pdrv, u32 *buffer, u32 offset, u32 nbytes);
int CfiFlashWrite(u32 pdrv, const u32 *buffer, u32 offset, u32 nbytes);
int CfiFlashErase(u32 pdrv, u32 offset);

#endif /* __CFIFLASH_H__ */
