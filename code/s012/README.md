# Show me the code --- S012.virtio-net

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



## virtio-net

> docs\virtio-v1.2.pdf   5.1 Network Device

virtio-net 是半虚拟网卡设备，通过它，qemu 可以让 guest os 收发网络报文，与其他主机通信。

virtio-net的 virtqueues 分为三种，分别为：

- 0 receiveq1
- 1 transmitq1
- 2  controlq

本示例只用到接收queue 和发送queue 

- 接收一个报文，需要使用一个 descriptor (描述符)，格式为：

  ```c
  struct virtio_net_rxpkt {
  #define VIRTIO_NET_HDR_F_NEEDS_CSUM 1
  #define VIRTIO_NET_HDR_F_DATA_VALID 2
  #define VIRTIO_NET_HDR_F_RSC_INFO 4
  	u8 flags;
  #define VIRTIO_NET_HDR_GSO_NONE 0
  #define VIRTIO_NET_HDR_GSO_TCPV4 1
  #define VIRTIO_NET_HDR_GSO_UDP 3
  #define VIRTIO_NET_HDR_GSO_TCPV6 4
  #define VIRTIO_NET_HDR_GSO_UDP_L4 5
  #define VIRTIO_NET_HDR_GSO_ECN 0x80
  	u8 gso_type;
  	u16 hdr_len;
  	u16 gso_size;
  	u16 csum_start;
  	u16 csum_offset;
  	//u16 num_buffers; (Only if VIRTIO_NET_F_MRG_RXBUFT negotiated)
  	//u32 hash_value; (Only if VIRTIO_NET_F_HASH_REPORT negotiated)
  	//u16 hash_report; (Only if VIRTIO_NET_F_HASH_REPORT negotiated)
  	//u16 padding_reserved; (Only if VIRTIO_NET_F_HASH_REPORT negotiated)
  #define VIRTIO_NET_PKT_LEN 1514
  	u8 pkt[VIRTIO_NET_PKT_LEN];
  };
  ```

- 发送一个报文，需要使用两个 descriptor  (描述符)，格式为：

  ```c
  struct virtio_net_txhdr {
  	u8 flags;
  	u8 gso_type;
  	u16 hdr_len;
  	u16 gso_size;
  	u16 csum_start;
  	u16 csum_offset;
  	//u16 num_buffers; (Only if VIRTIO_NET_F_MRG_RXBUFT negotiated)
  	//u32 hash_value; (Only if VIRTIO_NET_F_HASH_REPORT negotiated)
  	//u16 hash_report; (Only if VIRTIO_NET_F_HASH_REPORT negotiated)
  	//u16 padding_reserved; (Only if VIRTIO_NET_F_HASH_REPORT negotiated)
  };
  ```

  ```c
  u8 data[ /* len */];
  ```

  

## 演示

1. cd code/s012/

2. make clean

3. make

4. make qemu

5. 退出qemu: 

   **Ctrl** + **a** + **x**



## 主页

更多信息，请关注B站 **芯片人阿伟**：

https://space.bilibili.com/243180540



