# Show me the code --- S002.printf

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



## printf - 格式化打印函数

在系列视频 S001 中，我们介绍了 qemu-riscv-virt 的 NS16550 打印功能，并实现了相关代码。

但在实际工程中，往往需要更复杂的格式化打印接口 printf。

学过C语言的朋友，应该都对 printf 不陌生吧：

int printf(const char *format, ...);



在嵌入式开发中，printf 有多个实现：

1. newlib 一般会自带 printf 实现，但是占用RAM太多
2. 自己动手写个 printf，实现难度很大
3. 移植 github 上公开的 small printf 实现



## small printf

small printf，顾名思义，实现了一部分 printf 功能，满足嵌入式调试的打印需求，同时占用RAM很小。

目前找到的实现有这些：

- https://github.com/mpaland/printf
- https://github.com/heartoftechnology/embedded-printf
- https://github.com/eyalroz/printf

其中 eyalroz/printf 功能上比较全面，可通过 printf_config.h 配置 printf 功能

同时在速度和RAM占用上也比较有优势。

因此，我们这期使用的是基于 eyalroz/printf 的移植。



## 演示

首先简单讲解下新加的代码，然后演示

1. cd code/s002/

2. make clean; make

3. make qemu

4. 退出qemu: 

   **Ctrl** + **a** + **x**



## 主页

更多信息，请关注B站 **芯片人阿伟**：

https://space.bilibili.com/243180540



