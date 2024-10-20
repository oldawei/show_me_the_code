# Show me the code --- S036.FreeRTOS(9) 移植coreSNTP

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



## coreSNTP

> SNTP: Simple Network Time Protocol

github：https://github.com/FreeRTOS/coreSNTP

coreSNTP 是 FreeRTOS 官方出品的 SNTP 客户端C语言库，它支持从 NTP/SNTP 服务器端获取时间同步信息。

这个库是包括在 LTS 版本里面的，代码质量很高

移植指南：https://freertos.github.io/coreSNTP/main/sntp_porting.html

1. Logging Configuration Macros
2. DNS Resolve Function
3. UDP Transport Interface
4. Get Time Function
5. Set Time Function
6. Authentication Interface (可选)



## backoffAlgorithm

github：https://github.com/FreeRTOS/backoffAlgorithm

backoffAlgorithm 库是一个实用工具库，用于分隔同一数据块的反复重传，以避免网络拥塞。该库使用带抖动的指数回退算法计算重试网络操作（例如与服务器的网络连接失败）的回退时间段。

这个库是包括在 LTS 版本里面的，代码质量很高。



## 演示

1. cd code/s036/

2. ./build.sh qemu

   退出qemu: **Ctrl** + **a** + **x**

3. ./build.sh clean



## 主页

更多信息，请关注B站 **芯片人阿伟**：

https://space.bilibili.com/243180540



