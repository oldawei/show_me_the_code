#include <stddef.h>
#include "riscv.h"
#include "riscv-virt.h"
#include "cfiflash.h"
#include "printf/printf.h"

#define CFIFLASH_ONE_BANK_BITS          24

#define CFIFLASH_SEC_SIZE               512
#define CFIFLASH_SEC_SIZE_BITS          9
#define CFIFLASH_SECTORS                (CFIFLASH_CAPACITY / CFIFLASH_SEC_SIZE)

#define CFIFLASH_PAGE_SIZE              (2048 * 2)          /* fit QEMU of 2 banks */
#define CFIFLASH_PAGE_WORDS             (CFIFLASH_PAGE_SIZE / sizeof(u32))
#define CFIFLASH_PAGE_WORDS_MASK        (CFIFLASH_PAGE_WORDS - 1)

#define CFIFLASH_QUERY_CMD              0x98
#define CFIFLASH_QUERY_BASE             0x55
#define CFIFLASH_QUERY_QRY              0x10
#define CFIFLASH_QUERY_VENDOR           0x13
#define CFIFLASH_QUERY_SIZE             0x27
#define CFIFLASH_QUERY_PAGE_BITS        0x2A
#define CFIFLASH_QUERY_ERASE_REGION     0x2C
#define CFIFLASH_QUERY_BLOCKS           0x2D
#define CFIFLASH_QUERY_BLOCK_SIZE       0x2F
#define CFIFLASH_EXPECT_VENDOR          1       /* Intel command set */
#define CFIFLASH_EXPECT_PAGE_BITS       11
#define CFIFLASH_EXPECT_BLOCKS          127     /* plus 1: # of blocks, arm_virt is 255, riscv32 is 127 */
#define CFIFLASH_EXPECT_BLOCK_SIZE      512     /* times 128: block size */
#define CFIFLASH_EXPECT_ERASE_REGION    1

#define CFIFLASH_CMD_ERASE              0x20
#define CFIFLASH_CMD_CLEAR_STATUS       0x50
#define CFIFLASH_CMD_READ_STATUS        0x70
#define CFIFLASH_CMD_CONFIRM            0xD0
#define CFIFLASH_CMD_BUFWRITE           0xE8
#define CFIFLASH_CMD_RESET              0xFF

#define CFIFLASH_STATUS_READY_MASK      0x80

#define BIT_SHIFT8      8
#define BYTE_WORD_SHIFT 2

#define PRINT_ERR printf

static u32 g_cfiDrvBase[CFIFLASH_MAX_NUM];

static u8 *GetCfiDrvPriv(u32 pdrv)
{
    u8 *ret = NULL;
    if (pdrv >= 0 && pdrv < CFIFLASH_MAX_NUM) {
        ret = (u8 *)g_cfiDrvBase[pdrv];
    }

    if (ret == NULL) {
        PRINT_ERR("Get CfiFlash driver base failed!\n");
    }

    return ret;
}

static int SetCfiDrvPriv(u32 pdrv, u32 priv)
{
    g_cfiDrvBase[pdrv] = priv;
    return FLASH_OK;
}

u32 CfiFlashSec2Bytes(u32 sector)
{
    return sector << CFIFLASH_SEC_SIZE_BITS;
}

static inline u32 CfiFlashPageWordOffset(u32 wordOffset)
{
    return wordOffset & CFIFLASH_PAGE_WORDS_MASK;
}

static inline u32 CfiFlashEraseBlkWordAddr(u32 wordOffset)
{
    return wordOffset & CFIFLASH_ERASEBLK_WORDMASK;
}

static inline u32 W2B(u32 words)
{
    return words << BYTE_WORD_SHIFT;
}

static inline u32 B2W(u32 bytes)
{
    return bytes >> BYTE_WORD_SHIFT;
}

static inline int CfiFlashQueryQRY(u8 *p)
{
    u32 wordOffset = CFIFLASH_QUERY_QRY;

    if (p[W2B(wordOffset++)] == 'Q') {
        if (p[W2B(wordOffset++)] == 'R') {
            if (p[W2B(wordOffset)] == 'Y') {
                return FLASH_OK;
            }
        }
    }
    return FLASH_ERROR;
}

static inline int CfiFlashQueryUint8(u32 wordOffset, u8 expect, u8 *p)
{
    if (p[W2B(wordOffset)] != expect) {
        PRINT_ERR("[%s]name:0x%x value:%u expect:%u \n", __func__, wordOffset, p[W2B(wordOffset)], expect);
        return FLASH_ERROR;
    }
    return FLASH_OK;
}

static inline int CfiFlashQueryUint16(u32 wordOffset, u16 expect, u8 *p)
{
    u16 v;

    v = (p[W2B(wordOffset + 1)] << BIT_SHIFT8) + p[W2B(wordOffset)];
    if (v != expect) {
        PRINT_ERR("[%s]name:0x%x value:%u expect:%u \n", __func__, wordOffset, v, expect);
        return FLASH_ERROR;
    }
    return FLASH_OK;
}

static inline int CfiFlashIsReady(u32 wordOffset, u32 *p)
{
    dsb();
    p[wordOffset] = CFIFLASH_CMD_READ_STATUS;
    dsb();
    return p[wordOffset] & CFIFLASH_STATUS_READY_MASK;
}

/* all in word(4 bytes) measure */
static void CfiFlashWriteBuf(u32 wordOffset, const u32 *buffer, size_t words, u32 *p)
{
    u32 blkAddr = 0;

    /* first write might not be Page aligned */
    u32 i = CFIFLASH_PAGE_WORDS - CfiFlashPageWordOffset(wordOffset);
    u32 wordCount = (i > words) ? words : i;

    while (words) {
        /* command buffer-write begin to Erase Block address */
        blkAddr = CfiFlashEraseBlkWordAddr(wordOffset);
        p[blkAddr] = CFIFLASH_CMD_BUFWRITE;

        /* write words count, 0-based */
        dsb();
        p[blkAddr] = wordCount - 1;

        /* program word data to actual address */
        for (i = 0; i < wordCount; i++, wordOffset++, buffer++) {
            p[wordOffset] = *buffer;
        }

        /* command buffer-write end to Erase Block address */
        p[blkAddr] = CFIFLASH_CMD_CONFIRM;
        while (!CfiFlashIsReady(blkAddr, p)) { }

        words -= wordCount;
        wordCount = (words >= CFIFLASH_PAGE_WORDS) ? CFIFLASH_PAGE_WORDS : words;
    }

    p[0] = CFIFLASH_CMD_CLEAR_STATUS;
}

int CfiFlashQuery(u32 pdrv)
{
    u8 *p = GetCfiDrvPriv(pdrv);
    if (p == NULL) {
        return FLASH_ERROR;
    }
    u32 *base = (u32 *)p;
    base[CFIFLASH_QUERY_BASE] = CFIFLASH_QUERY_CMD;

    dsb();
    if (CfiFlashQueryQRY(p)) {
        PRINT_ERR("[%s: %d]not supported CFI flash : not found QRY\n", __func__, __LINE__);
        return FLASH_ERROR;
    }

    if (CfiFlashQueryUint16(CFIFLASH_QUERY_VENDOR, CFIFLASH_EXPECT_VENDOR, p)) {
        PRINT_ERR("[%s: %d]not supported CFI flash : unexpected VENDOR\n", __func__, __LINE__);
        return FLASH_ERROR;
    }

    if (CfiFlashQueryUint8(CFIFLASH_QUERY_SIZE, CFIFLASH_ONE_BANK_BITS, p)) {
        PRINT_ERR("[%s: %d]not supported CFI flash : unexpected BANK_BITS\n", __func__, __LINE__);
        return FLASH_ERROR;
    }

    if (CfiFlashQueryUint16(CFIFLASH_QUERY_PAGE_BITS, CFIFLASH_EXPECT_PAGE_BITS, p)) {
        PRINT_ERR("[%s: %d]not supported CFI flash : unexpected PAGE_BITS\n", __func__, __LINE__);
        return FLASH_ERROR;
    }

    if (CfiFlashQueryUint8(CFIFLASH_QUERY_ERASE_REGION, CFIFLASH_EXPECT_ERASE_REGION, p)) {
        PRINT_ERR("[%s: %d]not supported CFI flash : unexpected ERASE_REGION\n", __func__, __LINE__);
        return FLASH_ERROR;
    }

    if (CfiFlashQueryUint16(CFIFLASH_QUERY_BLOCKS, CFIFLASH_EXPECT_BLOCKS, p)) {
        PRINT_ERR("[%s: %d]not supported CFI flash : unexpected BLOCKS\n", __func__, __LINE__);
        return FLASH_ERROR;
    }

    if (CfiFlashQueryUint16(CFIFLASH_QUERY_BLOCK_SIZE, CFIFLASH_EXPECT_BLOCK_SIZE, p)) {
        PRINT_ERR("[%s: %d]not supported CFI flash : unexpected BLOCK_SIZE\n", __func__, __LINE__);
        return FLASH_ERROR;
    }

    base[0] = CFIFLASH_CMD_RESET;
    return FLASH_OK;
}

int CfiFlashInit(u32 pdrv, u32 priv)
{
    return SetCfiDrvPriv(pdrv, priv);
}

int CfiFlashRead(u32 pdrv, u32 *buffer, u32 offset, u32 nbytes)
{
    u32 i = 0;

    if ((offset + nbytes) > CFIFLASH_CAPACITY) {
        PRINT_ERR("flash over read, offset:%u, nbytes:%u\n", offset, nbytes);
        return FLASH_ERROR;
    }

    u8 *pbase = GetCfiDrvPriv(pdrv);
    if (pbase == NULL) {
        return FLASH_ERROR;
    }
    u32 *base = (u32 *)pbase;

    u32 words = B2W(nbytes);
    u32 wordOffset = B2W(offset);

    disable_irq();
    for (i = 0; i < words; i++) {
        buffer[i] = base[wordOffset + i];
    }
    enable_irq();
    return FLASH_OK;
}

int CfiFlashWrite(u32 pdrv, const u32 *buffer, u32 offset, u32 nbytes)
{
    if ((offset + nbytes) > CFIFLASH_CAPACITY) {
        PRINT_ERR("flash over write, offset:%u, nbytes:%u\n", offset, nbytes);
        return FLASH_ERROR;
    }

    u8 *pbase = GetCfiDrvPriv(pdrv);
    if (pbase == NULL) {
        return FLASH_ERROR;
    }
    u32 *base = (u32 *)pbase;

    u32 words = B2W(nbytes);
    u32 wordOffset = B2W(offset);

    disable_irq();
    CfiFlashWriteBuf(wordOffset, buffer, words, base);
    enable_irq();

    return FLASH_OK;
}

int CfiFlashErase(u32 pdrv, u32 offset)
{
    if (offset > CFIFLASH_CAPACITY) {
        PRINT_ERR("flash over erase, offset:%u\n", offset);
        return FLASH_ERROR;
    }

    u8 *pbase = GetCfiDrvPriv(pdrv);
    if (pbase == NULL) {
        return FLASH_ERROR;
    }
    u32 *base = (u32 *)pbase;

    u32 blkAddr = CfiFlashEraseBlkWordAddr(B2W(offset));

    disable_irq();
    base[blkAddr] = CFIFLASH_CMD_ERASE;
    dsb();
    base[blkAddr] = CFIFLASH_CMD_CONFIRM;
    while (!CfiFlashIsReady(blkAddr, base)) { }
    base[0] = CFIFLASH_CMD_CLEAR_STATUS;
    enable_irq();

    return FLASH_OK;
}