# Show me the code --- S014.aclint

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



## aclint

> docs\riscv-aclint-1.0-rc4.pdf

aclint - Advanced clint, 高级本地中断控制器，它相较于 clint，具有以下特点：

- 兼容 clint
- 提供 s-mode ipi (interprocessor interrupts, 核间中断)
- s-mode ipi 通过 mip/sip 寄存器的ssip清除

![11](docs\11.png)

**意义：**

clint 没有 s-mode ipi 的能力，aclint 具备此能力，可以为嵌入式 linux 系统提供更好的权限隔离功能。



## 演示

1. cd code/s014/

2. make clean

3. make

4. make qemu

5. 退出qemu: 

   **Ctrl** + **a** + **x**



## 主页

更多信息，请关注B站 **芯片人阿伟**：

https://space.bilibili.com/243180540



