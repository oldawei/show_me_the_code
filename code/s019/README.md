# Show me the code --- S019. PCIe(3)读取设备功能

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



## Capability List

PCIe 设备的功能列表，是表示该设备具有什么功能的，可通过 BAR 空间读取。

![19-0](docs\19-0.png)

1. 首先读取 bar 0x34，获取能力列表的基址

2. 按照以下格式解析能力列表：

   ```c
   struct virtio_pci_cap {
       u8 cap_vndr; /* Generic PCI field: PCI_CAP_ID_VNDR */
       u8 cap_next; /* Generic PCI field: next ptr. */
       u8 cap_len; /* Generic PCI field: capability length */
       u8 cfg_type; /* Identifies the structure. */
       u8 bar; /* Where to find it. */
       u8 id; /* Multiple capabilities of the same type */
       u8 padding[2]; /* Pad to full dword. */
       le32 offset; /* Offset within bar. */
       le32 length; /* Length of the structure, in bytes. */
   };
   ```

   

3. 打印设备的能力

<img src="docs\19-1.jpeg" alt="19-1"  />



通过这张图，我们可用知道设备有如下能力：

1. bar 1 是 msi-x 功能区
2. bar4 是 virtio pci 功能区
3. bar4 有 4 个功能单元，分别是：
   - COMMON_CFG
   - ISR_CFG
   - DEVICE_CFG
   - NOTIFY_CFG



## 演示

1. cd code/s019/

2. make clean

3. make

4. make qemu

5. 退出qemu: 

   **Ctrl** + **a** + **x**



## 主页

更多信息，请关注B站 **芯片人阿伟**：

https://space.bilibili.com/243180540



