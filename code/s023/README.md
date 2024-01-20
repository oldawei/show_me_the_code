# Show me the code --- S023. PCIe(7) MSI-X 中断

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



## MSI-X

MSI-X 是 MSI extended 的缩写，意为 “扩展的消息信号中断”

MSI-X 是从 PCIe 3.0 规范引入的，主要为了解决 MSI 存在的缺陷： 

- 每个 PF 最多 32个中断向量
- 每个 PF 的中断号必须连续
- 每个 PF 的 Message Address 相同，不支持中断的多核分发
- 每个 PF 的 Message Address 相同，不支持中断的多特权级分发

![03](docs\03.png)

在 qemu-virt-riscv 平台中，aia - imsic 是支持多核CPU的，并且支持M、S两个特权级。

如果只使用 PCIe MSI 中断机制，则不能很好的发挥 IMSIC 中断控制器的功能。





## MSI-X Capability Structure

MSI-x Capability Structure 可以总结为：

1个 Message Control + 2张 Table

![06](docs\04.png)

![04](docs\08.png)

**Message Control Register**，含义如下图所示：

![04](docs\05.png)

MSI-X 中存在两张表，即：MSI-X table 和 PBA table

MSI-X table 是需要系统配置的，设置设备的 PF Message Address，Message Data, Mask bit

PBA table 是PCIe 设备设置的，表示当前有哪些中断处于 Pending 状态

## 如何配置

BIR（Base address Indicator Register）BAR 指示器，用来表示 MSI-X table 和 PBA table 在哪个 bar 上。

MSI-X table Offset 表示 MSI-X table 在 bar 空间内的偏移地址，MSI-X table 格式如下：

![04](docs\06.png)

Message Address 为 64 bits，系统配置时这里填 aia - imsic mmio 基址（分M、S mode 基址）

Message Data 为 32 bits，系统配置时这里填 aia - imsic 分配的中断号（可不连续）

Mask bit 为 1 bit，设置为1 表示关闭当前中断



PBA Offset 表示 PBA table 在 bar 空间内的偏移地址：

![04](docs\07.png)



更多详情，可参考这篇博客：

https://blog.chinaaet.com/justlxy/p/5100057843



## 完整拼图





![04](docs\09.png)

## 演示

本期无演示内容



## 主页

更多信息，请关注B站 **芯片人阿伟**：

https://space.bilibili.com/243180540



