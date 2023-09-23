# Show me the code --- S001.串口打印

## 开发环境

不清楚如何搭建开发环境的朋友，请参看 S000 - 开发环境搭建：

https://www.bilibili.com/video/BV1Xm4y1V757/



## 代码仓库

github 和 gitee 双平台同步更新！

https://gitee.com/oldawei/show_me_the_code

https://github.com/oldawei/show_me_the_code



## 免责声明

切勿用于生产环境！

因使用不当造成的任何损失，本人概不担责！

本项目中的代码，只做学习使用，没有经过严格测试，不保证结果的准确性可靠性！



## 外设

The `qemu-riscv-virt` machine supports the following devices:

- Up to 8 generic RV32GC/RV64GC cores, with optional extensions
- Core Local Interruptor (CLINT)
- Platform-Level Interrupt Controller (PLIC)
- CFI parallel NOR flash memory
- 1 NS16550 compatible UART
- 1 Google Goldfish RTC
- 1 SiFive Test device
- 8 virtio-mmio transport devices
- 1 generic PCIe host bridge
- The fw_cfg device that allows a guest to obtain data from QEMU

各种外设的地址是：

```c
static const MemMapEntry virt_memmap[] = {
    [VIRT_DEBUG] =        {        0x0,         0x100 },
    [VIRT_MROM] =         {     0x1000,        0xf000 },
    [VIRT_TEST] =         {   0x100000,        0x1000 },
    [VIRT_RTC] =          {   0x101000,        0x1000 },
    [VIRT_CLINT] =        {  0x2000000,       0x10000 },
    [VIRT_ACLINT_SSWI] =  {  0x2F00000,        0x4000 },
    [VIRT_PCIE_PIO] =     {  0x3000000,       0x10000 },
    [VIRT_PLATFORM_BUS] = {  0x4000000,     0x2000000 },
    [VIRT_PLIC] =         {  0xc000000, VIRT_PLIC_SIZE(VIRT_CPUS_MAX * 2) },
    [VIRT_APLIC_M] =      {  0xc000000, APLIC_SIZE(VIRT_CPUS_MAX) },
    [VIRT_APLIC_S] =      {  0xd000000, APLIC_SIZE(VIRT_CPUS_MAX) },
    [VIRT_UART0] =        { 0x10000000,         0x100 },
    [VIRT_VIRTIO] =       { 0x10001000,        0x1000 },
    [VIRT_FW_CFG] =       { 0x10100000,          0x18 },
    [VIRT_FLASH] =        { 0x20000000,     0x4000000 },
    [VIRT_IMSIC_M] =      { 0x24000000, VIRT_IMSIC_MAX_SIZE },
    [VIRT_IMSIC_S] =      { 0x28000000, VIRT_IMSIC_MAX_SIZE },
    [VIRT_PCIE_ECAM] =    { 0x30000000,    0x10000000 },
    [VIRT_PCIE_MMIO] =    { 0x40000000,    0x40000000 },
    [VIRT_DRAM] =         { 0x80000000,           0x0 },
};
```

详情参看 qemu 源码：

https://github.com/qemu/qemu/blob/v7.2.5/hw/riscv/virt.c

其中本期实验使用的串口地址为：`[VIRT_UART0] =        { 0x10000000,         0x100 },`



## 串口

NS16550 是一种串口外设，关于它的具体寄存器资料，可参考：http://byterunner.com/16550.html

因为它简单易实现，在qemu中被大量使用，用于打印调试信息。

本次实验，主要实现以下 NS16550 接口：

- void UartInit(u32 base_addr)
- int UartGetc(void)
- int UartPutc(int c)
- int UartOut(int c)

具体代码我也不逐行解释了，感兴趣的自己看，或者B站、github上联系我探讨。



## 源码

- include  --- 公共头文件目录
- source --- 源代码目录
  - cpu--- cpu相关的源码，这里指 riscv cpu
  - driver --- 外设驱动源码，这里指串口、RTC、Flash等外设
  - library --- 库函数源码，这里指各种通用函数
  - platform --- 平台相关的源码，这里指 qemu-riscv-virt 平台

我们现在简单过一下代码！



## 演示

1. cd code/s001/

2. make clean; make

3. make qemu

4. 退出qemu: 

   **Ctrl** + **a** + **x**



## 主页

更多信息，请关注B站 **芯片人阿伟**：

https://space.bilibili.com/243180540



