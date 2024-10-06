# Show me the code --- S034.程序控制qemu

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



## 控制qemu

当程序发生异常时，往往需要退出qemu、或重启qemu。

![01](docs\01.png)

在 qemu risc-v virt 平台上，如何使用程序优雅的控制呢？



## sifive test

qemu risc-v virt 平台提供的测试设备，sifive test 设备。

该设备有一个寄存器，可控制 qemu 的退出，或重启行为：

- 失败退出，往寄存器写入 0x3333
- 正常退出，往寄存器写入 0x5555
- 重启qemu，往寄存器写入0x7777



## 演示

1.   cd code/s034/

2.   ./build.sh qemu

   退出qemu: **Ctrl** + **a** + **x**

3.   ./build.sh clean

## 主页

更多信息，请关注B站 **芯片人阿伟**：

https://space.bilibili.com/243180540



