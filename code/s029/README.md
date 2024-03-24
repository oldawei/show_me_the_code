# Show me the code --- S029.FreeRTOS(4) TCP/IP协议栈

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



## FreeRTOS-Plus-TCP

> https://www.freertos.org/zh-cn-cmn-s/FreeRTOS-Plus/FreeRTOS_Plus_TCP/index.html



**版本**：

- V3.1.0 （LTS）
- V4.0.0（只能与 freertos-v11.0.1 一起使用）
- V4.1.0 （只能与 freertos-v11.0.1 一起使用）



**特点**：

![01](docs\01.png)



**协议**: 与 FreeRTOS 一样，都遵循 MIT 协议发布



## virtio-mmio-net

> https://www.freertos.org/zh-cn-cmn-s/FreeRTOS-Plus/FreeRTOS_Plus_TCP/FreeRTOS_TCP_Porting.html

本次演示，使用的是 zero copy 的移植方案，更高效。

![Media_Driver](docs\02.png)

三个版本 V3.1.0、V4.0.0、V4.1.0 都有移植，具体可参考：

FreeRTOS-Plus-TCPV3.1.0\portable\NetworkInterface\virtio-mmio-net\NetworkInterface.c

FreeRTOS-Plus-TCPV4.0.0\portable\NetworkInterface\virtio-mmio-net\NetworkInterface.c

FreeRTOS-Plus-TCPV4.1.0\portable\NetworkInterface\virtio-mmio-net\NetworkInterface.c

总体移植难度较大！



## 演示

1. cd code/s029/

2. make clean

3. make

4. make qemu

5. 退出qemu: 

   **Ctrl** + **a** + **x**



## 主页

更多信息，请关注B站 **芯片人阿伟**：

https://space.bilibili.com/243180540



