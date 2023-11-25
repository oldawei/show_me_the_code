# Show me the code --- S017. PCIe(1)设备探测

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



## PCIe 是什么

> PCI-Express: peripheral component interconnect express

PCIe 是一种高速串行计算机扩展总线标准，属于高速串行点对点双通道高带宽传输，所连接的设备分配独享通道带宽，不共享总线带宽。

PCIe 常用的两种存在形式：

- M.2 接口通道，主要插支持 M.2 的固态硬盘

- PCIe 标准插槽，支持显卡、固态硬盘、无线网卡、有线网卡、声卡、视频采集卡

  

PCIe接口的总线带宽是按长度划分为：PCIe X1、PCIe X2、PCIe X4、PCIe X8、PCIe X16

<img src="docs\11.jpg" alt="11" style="zoom:80%;" />



## PCIe 速率

传输速率为每秒传输量 GT/s，而不是每秒位数 Gbps，因为传输量包括不提供额外吞吐量的开销位。

 比如 PCIe 1.x 和 PCIe 2.x 使用8b / 10b编码方案，导致占用了20% （= 2/10）的原始信道带宽。

GT/s —— Giga transation per second （千兆传输/秒），即每一秒内传输的次数。重点在于描述物理层通信协议的速率属性，可以不和链路宽度等关联。

Gbps —— Giga Bits Per Second （千兆位/秒）。

**GT/s 与Gbps 之间不存在成比例的换算关系。**

![](docs\12.png)

PCIe 吞吐量（可用带宽）计算方法：

**吞吐量 = 传输速率 *  编码方案**

以 PCIe 2.0 为例，理层协议中使用的是 8b/10b 的编码方案，因此每一条 Lane 支持 5 * 8 / 10 = 4 Gbps = 500 MB/s 的速率



## PCIe 空间

PCIe 主要有两种访问空间：

- 配置空间 - 通过 ecam 基址访问 PCIe 配置空间
- memory 空间 - 通过访问 PCIe mmio 基址访问 PCIe memory 空间



配置空间按照如下格式组织的：

![13](docs\13.png)

- Bus Number，第23~16位，为PCI设备的总线号
- Device Number，第15~11位，为PCI设备的设备号
- Function Number，第10~8位，为PCI设备的功能号
- Register Number，第7~2位，为PCI设备配置空间的寄存器号

在探测一个 PCIe 设备时，就是按照 bus、device、function 来计算 BAR 的首地址的：

```c
bar_addr = ecam_base_addr + (bus << 16) | (device<< 11) | (func << 8)
```



## PCIe BAR

> BAR - Base Address Register, 即基址寄存器

PCIe 设备的探测、基本配置都是通过 BAR 访问的，BAR 的组织格式如下：

![14](docs\14.png)

不同厂商的 PCIe 设备，它们的 Vendor ID、Device ID 是不同的，因此可通过它们探测特定的 PCIe 设备。





## 演示

1. cd code/s017/

2. make clean

3. make

4. make qemu

5. 退出qemu: 

   **Ctrl** + **a** + **x**



## 主页

更多信息，请关注B站 **芯片人阿伟**：

https://space.bilibili.com/243180540



