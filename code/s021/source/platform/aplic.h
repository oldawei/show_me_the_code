#ifndef APLIC_H_
#define APLIC_H_

#include "types.h"

#define APLIC_VIRTIO0_IRQ   0x01 // rng
#define APLIC_VIRTIO1_IRQ   0x02 // blk
#define APLIC_VIRTIO2_IRQ   0x03 // net
#define APLIC_UART0_IRQ     0x0a // uart
#define APLIC_PCIE0_IRQ     0x21 // pci-net
#define APLIC_IPI0_IRQ      0xff // ipi

enum {
    APLIC_MACHINE = 0,
    APLIC_SUPERVISOR = 1
};

enum {
    APLIC_DM_DIRECT = 0,
    APLIC_DM_MSI = 1
};

enum {
    APLIC_SM_INACTIVE = 0,
    APLIC_SM_DETACHED = 1,
    APLIC_SM_RISING_EDGE = 4,
    APLIC_SM_FALLING_EDGE = 5,
    APLIC_SM_LEVEL_HIGH = 6,
    APLIC_SM_LEVEL_LOW = 7
};

struct idc {
    u32 idelivery;
    u32 iforce;
    u32 ithreshold;
    u32 _reserved1[3];
    u32 topi;
    u32 claimi;
};

struct aplic {
    u32 domaincfg;
    u32 sourcecfg[1023];
    u8 _reserved1[0xBC0];

    u32 mmsiaddrcfg;
    u32 mmsiaddrcfgh;
    u32 smsiaddrcfg;
    u32 smsiaddrcfgh;
    u8 _reserved2[0x30];

    u32 setip[32];
    u8 _reserved3[92];

    u32 setipnum;
    u8 _reserved4[0x20];

    u32 in_clrip[32];
    u8 _reserved5[92];

    u32 clripnum;
    u8 _reserved6[32];

    u32 setie[32];
    u8 _reserved7[92];

    u32 setienum;
    u8 _reserved8[32];

    u32 clrie[32];
    u8 _reserved9[92];

    u32 clrienum;
    u8 _reserved10[32];

    u32 setipnum_le;
    u32 setipnum_be;
    u8 _reserved11[4088];

    u32 genmsi;
    u32 target[1023];
};

struct aplic *aplic_get_addr(int aplic_mode);
void aplic_set_domaincfg(struct aplic *aplic, int mode);
void aplic_set_sourcecfg(struct aplic *aplic, int irq, int mode);
void aplic_sourcecfg_delegate(struct aplic *aplic, int irq, int child);
void aplic_set_msiaddr(struct aplic *aplic, int mode, u32 msi_addr);
void aplic_set_ip(struct aplic *aplic, int irq, int pending);
void aplic_set_ie(struct aplic *aplic, int irq, int enable);
void aplic_set_target_direct(struct aplic *aplic, int irq, int hart, int prio);
void aplic_set_target_msi(struct aplic *aplic, int irq, int hart, int guest, int eiid);
void aplic_set_idc(struct aplic *aplic, int hart, int idelivery, int ithreshold);
int aplic_get_claimi(int aplic_mode, int hart);
void aplic_enable_irq(int aplic_mode, int dm_mode, int irq, int enable);
void aplic_enable(int irq);
void aplic_init(int mode);


#endif /* APLIC_H_ */
