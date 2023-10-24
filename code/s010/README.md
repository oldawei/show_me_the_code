# Show me the code --- S010.virtio-blk

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



## virtio-blk

> docs\virtio-v1.2.pdf   5.2 Block Device

virtio-blk 是一种块存储设备，相当于 disk（硬盘）设备，一个 sector 为512字节。

通过 virtio-blk，qemu 为 guest 提供了与 host 交换数据的通道。

virtio-blk 中的 virtqueues 类型都是 requestq （请求队列），读/写磁盘的操作都是通过同一个 requestq 进行的。

不区分读操作 queue、写操作 queue，一次读/写操作，需要使用三个 descriptor (描述符)，分别填充为：

- request header，请求头

  ```c
  struct virtio_blk_req {
  #define VIRTIO_BLK_T_IN     0 // read the disk
  #define VIRTIO_BLK_T_OUT    1 // write the disk
    u32 type;
    u32 reserved;
    u64 sector; // one sector are 512 bytes
  };
  ```

  

- data，len，读/写数据

  ```c
  u8 data[ /* len */];
  ```

  

- status，读/写状态

  ```c
  define VIRTIO_BLK_S_OK 0
  #define VIRTIO_BLK_S_IOERR 1
  #define VIRTIO_BLK_S_UNSUPP 2
  u8 status;
  ```



## 演示

1. cd code/s010/

2. make clean

3. make

4. make qemu

5. 退出qemu: 

   **Ctrl** + **a** + **x**



## 主页

更多信息，请关注B站 **芯片人阿伟**：

https://space.bilibili.com/243180540



