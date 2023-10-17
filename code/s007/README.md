# Show me the code --- S007.virtio简介

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



## virtio

> virtio v1.2 spec：docs\virtio-v1.2.pdf

virtio 是一种半虚拟化硬件设备的技术，主要用于解决全虚拟化硬件设备导致的性能瓶颈，以及热迁移等问题。

全虚拟化 vs 半虚拟化：

![](img\11.gif)

- 全虚拟化的 guest 不知道自己跑在虚拟化环境中，使用原生设备驱动，如e1000网卡驱动，与 qemu 交互时采用 traps，效率低
- 采用 virtio 技术时 guest 明确知道自己跑在虚拟化环境中，使用 virtio 设备驱动，与 qemu 交互时采用 virtqueues，效率高



半虚拟化驱动架构:

![](img\10.gif)



virtio 大致上可以分为三个层次，分别是：

- guest 驱动层 （跑在 qemu-riscv-virt 虚拟机里面的）
- transport 层 （同时跑在 virt 虚拟机和 qemu 进程里面的）
- 硬件设备层 （跑在 qemu 进程里，或者 kvm 里）

![](img\11.png)



## virtqueues

> 技术细节参考 docs\virtio-v1.2.pdf, 2.7 Split Virtqueues  

在 virtio 中，前端和后端通过 virtqueues 进行通信。virtqueues又分为 split virtqueues 和 packed virtqueues 。

因为我们使用的 virtio 硬件设备只支持 split virtqueues ，因此这里只介绍下 split virtqueues 的组成。

split virtqueues 有三部分组成：

- Descriptor Table - occupies the Descriptor Area （4K地址对齐）

  ```c
  struct virtq_desc {
      /* Address (guest-physical). */
      le64 addr;
      /* Length. */
      le32 len;
      /* This marks a buffer as continuing via the next field. */
  #define VIRTQ_DESC_F_NEXT 1
      /* This marks a buffer as device write-only (otherwise device read-only). */
  #define VIRTQ_DESC_F_WRITE 2
      /* This means the buffer contains a list of buffer descriptors. */
  #define VIRTQ_DESC_F_INDIRECT 4
      /* The flags as indicated above. */
      le16 flags;
      /* Next field if flags & NEXT */
      le16 next;
  };
  ```

  

- Available Ring - occupies the Driver Area（4K地址对齐）

  ```c
  struct virtq_avail {
  #define VIRTQ_AVAIL_F_NO_INTERRUPT 1
      le16 flags;
      le16 idx;
      le16 ring[ /* Queue Size */ ];
      le16 used_event; /* Only if VIRTIO_F_EVENT_IDX */
  };
  ```

  

- Used Ring - occupies the Device Area  （4K地址对齐）

  ```c
  /* le32 is used here for ids for padding reasons. */
  struct virtq_used_elem {
      /* Index of start of used descriptor chain. */
      le32 id;
      /*
      * The number of bytes written into the device writable portion of
      * the buffer described by the descriptor chain.
      */
      le32 len;
  };
  
  struct virtq_used {
  #define VIRTQ_USED_F_NO_NOTIFY 1
      le16 flags;
      le16 idx;
      struct virtq_used_elem ring[ /* Queue Size */];
      le16 avail_event; /* Only if VIRTIO_F_EVENT_IDX */
  };
  ```



- split virtqueues 结构体：

  ```c
  struct virtq {
      // The actual descriptors (16 bytes each)
      struct virtq_desc *desc; 	// 4k aligned address
      // A ring of available descriptor heads with free-running index.
      struct virtq_avail *avail; 	// 4k aligned address
      // A ring of used descriptor heads with free-running index.
      struct virtq_used *used; 	// 4k aligned address
  };
  ```

  

前端与后端的通信方式：

<img src="img\23.png" style="zoom:70%;" />



带 split virtqueues 的架构图：

<img src="img\12.png" style="zoom:80%;" />

技术细节可参考：https://www.redhat.com/en/blog/virtqueues-and-virtio-ring-how-data-travels



## 演示

1. cd code/s007/

2. make clean

3. make

4. make qemu

5. 退出qemu: 

   **Ctrl** + **a** + **x**



## 主页

更多信息，请关注B站 **芯片人阿伟**：

https://space.bilibili.com/243180540



