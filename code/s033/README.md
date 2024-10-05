# Show me the code --- S033.估算 risc-v 主频

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



## Mtimer clock

qemu risc-v virt 平台的 mtimer 时钟主频是 10MHz 的，

但这只是 qemu 官方给的参数，

有没有方法在程序里自动计算出来呢？

测量工具：

- goldfish rtc - 读取时间戳
- mtimer - 读取 mtimer 运行的周期数



## Cpu clock

qemu risc-v virt 平台在 win10 上运行的 cpu 主频是多少?

官方文档并没有给出具体的数值，很好奇有没有方法计算出来？

测量工具：

- goldfish rtc - 读取时间戳
- rdcycle - 读取 cpu 运行的周期数



## 演示

1.   cd code/s033/

2.   ./build.sh qemu

   退出qemu: **Ctrl** + **a** + **x**

3.   ./build.sh clean

## 主页

更多信息，请关注B站 **芯片人阿伟**：

https://space.bilibili.com/243180540



