# Show me the code --- S009.virtio-rng

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



## virtio-rng

> 技术细节参考 docs\virtio-v1.2.pdf, 5.4 Entropy Device  

Entropy Device 是 virtio 中比较简单的一种设备，主要用于为 qemu guest 提供随机数。

驱动主要分3部分：

- virtio_mmio_init ： mmio 初始化

- virtio_rng_init ：rng 设备初始化
- virtio_rng_read ：读取随机数数据

接下来简单讲解下 C 代码



## 演示

1. cd code/s009/

2. make clean

3. make

4. make qemu

5. 退出qemu: 

   **Ctrl** + **a** + **x**



## 主页

更多信息，请关注B站 **芯片人阿伟**：

https://space.bilibili.com/243180540



