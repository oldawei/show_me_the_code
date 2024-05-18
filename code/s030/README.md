# Show me the code --- S030.FreeRTOS(5) FTP/HTTP

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



## FTP/HTTP

> https://github.com/FreeRTOS/FreeRTOS/tree/main/FreeRTOS-Plus/Demo/Common/Demo_IP_Protocols



主要组件：

- FreeRTOS
- FreeRTOS-Plus-CLI
- FreeRTOS-Plus-TCP
- FreeRTOS-Plus-FAT



同时提供 FTP server 与 HTTP server

ftp://192.168.123.2:2121/

http://192.168.123.2:8080/



## 演示

1. cd code/s030/

2. make clean

3. make

4. make qemu

5. 退出qemu: 

   **Ctrl** + **a** + **x**



## 主页

更多信息，请关注B站 **芯片人阿伟**：

https://space.bilibili.com/243180540



