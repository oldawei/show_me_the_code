# Show me the code --- S003.CLINT

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



## CLINT- Core Local Interrupt Controller

CLINT是 risc-v 本地中断控制器，用于控制定时器、软中断的

| Address           | Width | Attr | Name      | Description                             |
| ----------------- | ----- | ---- | --------- | --------------------------------------- |
| `BASE` + `0x0000` | 4B    | RW   | MSIP0     | HART index 0 machine-level IPI register |
| `BASE` + `0x0004` | 4B    | RW   | MSIP1     | HART index 1 machine-level IPI register |
| …                 | …     | …    | …         | …                                       |
| `BASE` + 0x3FFC   | 4B    |      | RESERVED  | Reserved for future use.                |
| `BASE` + `0x4000` | 8B    | RW   | MTIMECMP0 | HART index 0 machine-level time compare |
| `BASE` + `0x4008` | 8B    | RW   | MTIMECMP1 | HART index 1 machine-level time compare |
| …                 | …     | …    | …         | …                                       |
| `BASE` + `0xBFF8` | 8B    | RW   | MTIME     | Machine-level time counter              |

具体细节可参考文档：**docs\riscv-aclint-1.0-rc4.pdf**



## Machine-level Software Interrupt Device

Each MSIP register is a 32-bit wide WARL register where the upper 31 bits are wired to zero. The least significant bit is reflected in MSIP of the `mip` CSR. A machine-level software interrupt for a HART is pending or cleared by writing `1` or `0` respectively to the corresponding MSIP register.

On MSWI device reset, each MSIP register is cleared to zero.

**软中断是cpu核间通信的重要手段，例如：**

**CPU0、CPU1共用一块BUFF。CPU0向BUFF写数据，CPU1从BUFF读数据。**

**CPU0写完数据后，可以设置MSIP1中断，CPU1收到中断后及时处理BUFF数据。**



## Machine-level Timer Device

The MTIMER device provides machine-level timer functionality for a set of HARTs on a RISC-V platform. It has a single fixed-frequency monotonic time counter (**MTIME**) register and a time compare register (**MTIMECMP**) for each HART connected to the MTIMER device. A MTIMER device not connected to any HART should only have a MTIME register and no MTIMECMP registers.

On a RISC-V platform with multiple MTIMER devices:

- Each MTIMER device provides machine-level timer functionality for a different (or disjoint) set of HARTs. A MTIMER device assigns a HART index starting from zero to each HART associated with it. The HART index assigned to a HART by the MTIMER device may or may not have any relationship with the unique HART identifier (**hart ID**) that the RISC-V Privileged Architecture assigns to the HART.
- Two or more MTIMER devices can share the same physical MTIME register while having their own separate MTIMECMP registers.
- The MTIMECMP registers of a MTIMER device must only compare against the MTIME register of the same MTIMER device for generating machine-level timer interrupt.

The maximum number of HARTs supported by a single MTIMER device is 4095 which is equivalent to the maximum number of MTIMECMP registers.

**Mtime、Mtimecmp 主要用于CPU系统时钟，在RTOS中一般作为心跳时钟。**



## 演示

1. cd code/s003/

2. make clean; make

3. make qemu

4. 退出qemu: 

   **Ctrl** + **a** + **x**



## 主页

更多信息，请关注B站 **芯片人阿伟**：

https://space.bilibili.com/243180540



