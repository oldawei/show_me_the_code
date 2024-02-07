# Show me the code --- S027.FreeRTOS(2) 命令行接口CLI

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



## FreeRTOS CLI

网址：http://www.FreeRTOS.org/cli

FreeRTOS-Plus-CLI-V1.0.4

FreeRTOS-Plus-CLI （命令行接口）提供了一种简单、小巧、可扩展且 RAM 高效的启用方法 方便您的 FreeRTOS 应用程序处理命令行输入。 添加命令所需的步骤如下图表所示:

![11](docs\11.png)



## 演示

1. cd code/s027/

2. make clean

3. make

4. make qemu

5. 退出qemu: 

   **Ctrl** + **a** + **x**



help

task-stats

run-time-stats

query-heap

mem-read 80100000 10

mem-write 80100000 1234abcd

mem-read 80100000 10





## 主页

更多信息，请关注B站 **芯片人阿伟**：

https://space.bilibili.com/243180540



