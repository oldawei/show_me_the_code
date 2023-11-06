# Show me the code --- S013. device tree 设备树

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



## device tree

qemu-riscv-virt 平台有很多外设设备，不熟悉的情况下，如何快速了解它的外设地址、中断等情况呢？

答案是：设备树(device tree)

设备树，是一种描述硬件资源的数据结构，它通过bootloader将硬件资源传给内核，使得内核和硬件资源描述相对独立

Device Tree可以描述的信息包括：

- CPU的数量和类别
- 内存基地址和大小
- 总线和桥
- 外设连接
- 中断控制器和中断使用情况
- GPIO控制器和GPIO使用情况
- Clock控制器和Clock使用情况

使用设备树的优势：

对于使用同一 SoC 的不同主板，只需更换设备树文件.dtb 即可实现不同主板的无差异支持，而无需更换内核文件



设备树包含DTC（device tree compiler），DTS（device tree source）和 DTB（device tree blob）：

- dtc 设备树编译器
- dts 设备树源文件
- dtb 设备树二进制文件

通过 dtc，可用把 dts 编译成 dtb；也可用把 dtb 反编译成 dts

在 qemu 中，可用通过 dumpdtb=file.dtb 参数导出当前虚拟机的 dtb 文件



## 演示

1. 导出 dtb 文件:

   ```bash
   make qemu
   ```

2. 运行 ubuntu

   ```bash
   docker run -it --rm -v ./:/jobs ubuntu:22.04 bash
   或者
   podman run -it --rm -v ./:/jobs ubuntu:22.04 bash
   ```

3. 安装 dtc

   ```bash
   cd /jobs
   cp sources.list /etc/apt/sources.list
   apt update
   apt install device-tree-compiler -y
   ```

   

4. 反编译 dtb 文件

   ```bash
   dtc -O dts -I dtb file.dtb -o file.dts
   ```



最后得到设备树的源文件：file.dts



## 主页

更多信息，请关注B站 **芯片人阿伟**：

https://space.bilibili.com/243180540



