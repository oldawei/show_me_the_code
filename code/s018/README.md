# Show me the code --- S018. PCIe(2)BAR设置

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



## PCIe BAR

BAR大小的确定：

1. 向 BAR 写入全F
2. 读取 BAR 的值，并清除 bit0-3的值
3. 将值取反加+1

```
u32 *pt_bar = ....;
*pt_bar = 0xFFFFFFFF;
u32 sz = *pt_bar & 0xFFFFFFF0;
sz = ~sz + 1;

```

**32 bit 系统上的 meomry BAR：**

![20](docs\20.png)

- bit 0: 表示 BAR 空间的类型，0 为 memory 空间，1为 IO 空间
- bit 1-2：表示 BAR 访问的类型，00 表示 32-bit 访问空间，01 表示 64-bit 访问空间
-  bit 3：表示预取类型，0 表示不可预取，1表示可预取



**32 bit 系统上的 IO BAR：**

![20](docs\22.png)



**在 64 位系统 上，相邻的两个 BAR 合并使用，组成 64 bit 地址：**

![21](docs\21.png)



## 演示

1. cd code/s018/

2. make clean

3. make

4. make qemu

5. 退出qemu: 

   **Ctrl** + **a** + **x**



## 主页

更多信息，请关注B站 **芯片人阿伟**：

https://space.bilibili.com/243180540



