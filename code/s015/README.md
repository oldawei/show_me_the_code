# Show me the code --- S015.aia part1 - aplic

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



## aia

> docs\riscv-interrupts-1.0.pdf

aia is Advanced Interrupt Architecture for RISC-V，即 risc-v 高级中断架构。

它由两部分组成：

- APLIC (Advanced Platform-Level Interrupt Controller ) 高级平台级中断控制器
- IMSIC ( Incoming MSI Controller ) 消息信号中断控制器



aplic 可以独立工作，也可以配合 imsic 组成更高级的中断架构，支持 pcie msi/msi-x 中断的处理。

![11](docs\11.png)

**意义：**

- aplic 提供了多核 ipi，以及 m-mode、s-mode 等权限的隔离功能
- aplic + imsic 可提供 msi、msi-x 中断支持，这个对 pcie 设备非常重要

## aplic

> docs\riscv-interrupts-1.0.pdf
>
> Chapter 4.Advanced Platform-Level Interrupt Controller (APLIC)

aplic 相较于 plic，具有以下特点：

- aplic 支持 m-mode、s-mode 等不同特权级的中断，从而实现中断的特权级隔离

  ![22](docs\22.png)

-  支持多domain、多 hart，配置灵活

  ![22](docs\33.png)

- 每个 domain 有单独的 Memory-mapped 控制地址

- 支持中断转换为 MSIs (Message Signaled Interrupts，消息信号中断)

- 支持基于 hart 的 IDC (Interrupt delivery control)，每个 hart 可单独打开中断/关闭中断



更多技术细节，可参考blog：

https://blog.stephenmarz.com/2022/07/26/aplic/

https://blog.stephenmarz.com/2022/06/30/msi/



## 演示

1. cd code/s015/

2. make clean

3. make

4. make qemu

5. 退出qemu: 

   **Ctrl** + **a** + **x**



## 主页

更多信息，请关注B站 **芯片人阿伟**：

https://space.bilibili.com/243180540



