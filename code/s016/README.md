# Show me the code --- S016.aia part2 - imsic

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



## IMSIC

> docs\riscv-interrupts-1.0.pdf 
>
> Chapter 3.Incoming MSI Controller (IMSIC) 

IMSIC ( Incoming MSI Controller ) 消息信号中断控制器，是 riscv aia 的重要组成部分。

IMSIC 的作用：

- 提供基于 MSI 的中断机制
- 提供核间 ipi 机制
- 为M、S mode 提供中断隔离
- 为 pcie 等设备提供 msi、msi-x 中断支持



imsic 是通过 CSRs 寄存器间接访问的，具体如下：

| Register Name | Register Number | Description                                              |
| :------------ | :-------------- | :------------------------------------------------------- |
| MISELECT      | 0x350           | Machine register select                                  |
| SISELECT      | 0x150           | Supervisor register select                               |
| MIREG         | 0x351           | A R/W view of the selected register in MISELECT          |
| SIREG         | 0x151           | A R/W view of the selected register in SISELECT          |
| MTOPI         | 0xFB0           | Machine top-level interrupt                              |
| STOPI         | 0xDB0           | Supervisor top-level interrupt                           |
| MTOPEI        | 0x35C           | Machine top-level external interrupt (requires IMSIC)    |
| STOPEI        | 0x15C           | Supervisor top-level external interrupt (requires IMSIC) |

MISELECT、SISELECT 可以配置的值为：

| Name               | MISELECT/SISELECT | Description                           |
| :----------------- | :---------------- | :------------------------------------ |
| EIDELIVERY         | 0x70              | External Interrupt Delivery Register  |
| EITHRESHOLD        | 0x72              | External Interrupt Threshold Register |
| EIP0 through EIP63 | 0x80 through 0xBF | External Interrupt Pending Registers  |
| EIE0 through EIE63 | 0xC0 through 0xFF | External Interrupt Enable Registers   |

EIDELIVERY :

| Value       | Description                                     |
| :---------- | :---------------------------------------------- |
| 0           | Interrupt delivery is disabled                  |
| 1           | Interrupt delivery is enabled                   |
| 0x4000_0000 | Optional interrupt delivery via a PLIC or APLIC |



IMSIC 还提供了 memory-mapped 空间，可直接写改空间来触发中断。这也是 MSI 的工作方式：

M mode MMIO 地址：

- 0x2400_0000 (hart 0)
- 0x2400_1000 (hart 1)
- 0x2400_2000 (hart 2)
- 0x2400_3000 (hart 3)



S mode MMIO 地址：

- 0x2800_0000 (hart 0)
- 0x2800_1000 (hart 1)
- 0x2800_2000 (hart 2)
- 0x2800_3000 (hart 3)



更多技术细节，可参考blog：

https://blog.stephenmarz.com/2022/07/26/aplic/

https://blog.stephenmarz.com/2022/06/30/msi/



## 演示

1. cd code/s016/

2. make clean

3. make

4. make qemu

5. 退出qemu: 

   **Ctrl** + **a** + **x**



## 主页

更多信息，请关注B站 **芯片人阿伟**：

https://space.bilibili.com/243180540



