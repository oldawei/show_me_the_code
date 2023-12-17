# Show me the code --- S021. PCIe(5)virtio-net驱动

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



## virtio-net 驱动

我们在本系列视频第12期，已经讲过基于 MMIO 传输层的 virtio-net 设备。

这期我们简单说下基于 pcie 传输层的 virtio-net 设备。

区别：

- 基于 MMIO 传输层的 virtio 设备，是通过 MMIO 基址去寻址配置设备的
- 基于 pcie 传输层的 virtio 设备，是通过 bar 空间划分的功能区去寻址配置设备的

具体到 virtio-net 设备：

1. bar 1 是 msi-x 功能区
2. bar4 是 virtio 功能区
3. bar4 有 4 个功能单元，分别是：
   - COMMON_CFG
   - ISR_CFG
   - DEVICE_CFG
   - NOTIFY_CFG



本期驱动代码中 virtio-net 中断使用的还是传统中断，INT#x

基于 MSI-x 中断的 virtio-net 驱动，将在后续详说



## 演示

1. cd code/s021/

2. make clean

3. make

4. make qemu

5. 退出qemu: 

   **Ctrl** + **a** + **x**



## 主页

更多信息，请关注B站 **芯片人阿伟**：

https://space.bilibili.com/243180540



