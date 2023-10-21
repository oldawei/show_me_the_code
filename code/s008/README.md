# Show me the code --- S008.virtio-mmio

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



## virtio-mmio

>  技术细节请参考 docs\virtio-v1.2.pdf，4.2 Virtio Over MMIO  

virtio-mmio 是 virtio 技术定义的三种传输层协议中的一种，也是最简单的一种。它主要是通过 mmio 接口来访问 virtio 硬件设备的。

它的寄存器规范在 virtio spec v1.2 **4.2.2 MMIO Device Register Layout  ** 中有明确定义，这里我简单讲下。

qemu 中的设计架构图：

**host**：跑在物理机器上的操作系统

**qemu：host**上的用户进程

**guest**：跑在qemu中的操作系统

**virtio-driver**: guest 上的内核驱动

**virtio-mmio device**：qemu 进程创建的virtio设备



<img src="img\22.png" style="zoom:70%;" />



## 演示

1. cd code/s008/

2. make clean

3. make

4. make qemu

5. 退出qemu: 

   **Ctrl** + **a** + **x**



## 主页

更多信息，请关注B站 **芯片人阿伟**：

https://space.bilibili.com/243180540



