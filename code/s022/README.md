# Show me the code --- S022. PCIe(6) MSI 中断介绍

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



## MSI

MSI - Message Signaled Interrupts, 即消息信号中断

MSI 机制的引入解决了传统 Line-based Interrupt 机制的种种限制，包括：

- 无需经过I/O APIC转发中断，直接通过 PCIe Memory Write Transaction 向 CPU 发送中断，效率更高
- 每个 PCI Function 可以支持分配多个中断向量，满足同一个设备有多个不同中断请求的需求
- 当分配多个中断向量给同1个 PCI Function 时，提供按中断向量进行屏蔽的功能，更为灵活



尽管1个PCIe设备可能同时支持Virtual INTx#/MSI/MSI-X，系统软件必须只Enable其中一种中断机制。

![img](docs\01.png)



根据 PCIe 规范的定义，MSI/MSI-X 中断请求发生时，PCIe 设备会实际产生1个 Memory Write Transaction。它对应的数据封包为Memory Write 类型的 Trasaction Layer Packet (TLP)，其格式如下图所示（对MSI-X也同样适用)。其中目标 Memory Address 称为“Message Address”，要写入该内存地址的数据称为 “Message Data”。这两个字段都来自MSI/MSI-X Capability Structure 中的设定，系统软件在Enable并初始化MSI/MSI-X的过程中必须要预先设置好它们。

![02](docs\02.png)



## MSI Capability Structure

MSI Capability Structure 有四种类型：

普通的：

![06](docs\06.png)

带 Mask 功能的：

![03](docs\03.png)



**Message Control Register**用于确定MSI的格式与支持的功能等信息，如下图所示：

![04](docs\04.png)



具体含义，看这张图：

![05](docs\05.png)



Message Address: aia - imsic mmio 基址

Message Data: aia - imsic 中断号



更多详情，可参考这篇博客：

https://blog.chinaaet.com/justlxy/p/5100057842

https://zhuanlan.zhihu.com/p/407100254

https://zhuanlan.zhihu.com/p/517861200







## 演示

本期无演示内容



## 主页

更多信息，请关注B站 **芯片人阿伟**：

https://space.bilibili.com/243180540



