# Show me the code --- S031.FreeRTOS(6) UDP组播

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



## UDP组播

IP 组播通信必须依赖于 IP 组播地址，在 IPv4 中它是一个 D 类 IP 地址，范围从 224.0.0.0 到 239.255.255.255，并被划分为3类：

- 局部链接组播地址，224.0.0.0~224.0.0.255，这是为路由协议和其它用途保留的地址，路由器并不转发属于此范围的IP包；
- 预留组播地址， 224.0.1.0~238.255.255.255，可用于全球范围（如Internet）或网络协议； 
- 管理权限组播地址， 239.0.0.0~239.255.255.255，可供组织内部使用，类似于私有 IP 地址，不能用于 Internet，可限制多播范围。



## Npcap

Npcap 是新一代的网络抓包工具，可以取代老旧的 Winpcap。广泛应用于Win10、Win11等系统。

本次演示，我们使用 Npcap 编写了一个小程序 UDPdump，用于抓取 UDP 组播报文。



## 演示

本次演示使用的组播地址为：239.10.20.30:9999

1. cd code/s031/

2. 打开 Npcap/UDPdump.exe

3. make clean

4. make

5. make qemu

6. 退出qemu: 

   **Ctrl** + **a** + **x**



## 主页

更多信息，请关注B站 **芯片人阿伟**：

https://space.bilibili.com/243180540



