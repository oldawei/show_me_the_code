# Show me the code --- S004.PLIC

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



## PLIC

PLIC(platform-level interrupt controller)，**平台级中断控制器**。用来将外部的全局中断请求处理后转至中断目标。![11](D:\gitee\show_me_the_code\code\s004\img\11.PNG)

## PLIC 寄存器

PLIC 的寄存器比较多，具体可参考 spec：

docs\riscv-plic-1.0.0.pdf

https://github.com/riscv/riscv-plic-spec/blob/master/riscv-plic-1.0.0.pdf

大概分为6大类别：

- Interrupt Priorities registers: The interrupt priority for each interrupt source.
- Interrupt Pending Bits registers: The interrupt pending status of each interrupt source.
- Interrupt Enables registers: The enablement of interrupt source of each context.
- Priority Thresholds registers: The interrupt priority threshold of each context.
- Interrupt Claim registers: The register to acquire interrupt source ID of each context.
- Interrupt Completion registers: The register to send interrupt completion message to the associated gateway.

![33](D:\gitee\show_me_the_code\code\s004\img\33.PNG)



## 中断处理流程



![22](D:\gitee\show_me_the_code\code\s004\img\22.PNG)



## 演示

1. cd code/s004/

2. make clean; make

3. make qemu

4. 退出qemu: 

   **Ctrl** + **a** + **x**



## 主页

更多信息，请关注B站 **芯片人阿伟**：

https://space.bilibili.com/243180540



