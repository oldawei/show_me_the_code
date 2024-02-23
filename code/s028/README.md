# Show me the code --- S028.FreeRTOS(3) FAT 文件系统

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



## FreeRTOS-Plus-FAT

> https://www.freertos.org/zh-cn-cmn-s/FreeRTOS-Plus/FreeRTOS_Plus_FAT/index.html



**版本**：V2.3.3

**特点**：

![01](docs\01.png)



**协议**: 与 FreeRTOS 一样，都遵循 MIT 协议发布

**专利问题**：FAT 文件系统是微软的专利，使用到商业产品中可能存在专利问题，请小心使用！



## virtio-mmio-blk

> https://www.freertos.org/zh-cn-cmn-s/FreeRTOS-Plus/FreeRTOS_Plus_FAT/Creating_a_file_system_media_driver.html

本次演示，是在 fat 文件系统下使用 virtio-mmio-blk 设备驱动的读写接口，实现磁盘读写的。

![Media_Driver](docs\Media_Driver.png)

FreeRTOS-Plus-FAT 需要实现的移植接口只需三个：

1. 从磁盘读取扇区的函数
2. 将扇区写入磁盘的函数
3. 磁盘设备初始化函数



个人感觉移植的难度小于 Fatfs 。关于 Fatfs 的使用，后续有机会再将！



## 演示

1. cd code/s028/

2. make clean

3. make

4. make qemu

5. 退出qemu: 

   **Ctrl** + **a** + **x**



## 主页

更多信息，请关注B站 **芯片人阿伟**：

https://space.bilibili.com/243180540



