# Show me the code --- S026.FreeRTOS(1) 移植

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



## FreeRTOS 移植

- FreeRTOS [V10.5.1 LTS](https://github.com/FreeRTOS/FreeRTOS-Kernel/releases/tag/V10.5.1)
- FreeRTOS [V11.0.1 Latest](https://github.com/FreeRTOS/FreeRTOS-Kernel/releases/tag/V11.0.1)

FreeRTOS 在 qemu virt 平台上的移植：

官方已经做好移植工作了，相关文件在目录：**portable\GCC\RISC-V**

需要注意的是 **chip_specific_extensions**，这里要选择：**RISCV_MTIME_CLINT_no_extensions**



移植比较关键的文件是 portASM.S，这里面提供了两种中断处理方式：

1.  Direct 模式：

   freertos_risc_v_trap_handler

   

2. Vectored 模式：(vector.S)

   freertos_risc_v_exception_handler

   freertos_risc_v_mtimer_interrupt_handler

   freertos_risc_v_interrupt_handler

   

另外还有两个中断 api 需要在项目里面实现：(isr.S)

freertos_risc_v_application_exception_handler

freertos_risc_v_application_interrupt_handler



hook.c:

FreeRTOS 需要的一些 hook 函数，主要用于错误处理



FreeRTOSConfig.h

FreeRTOS 配置文件，用于打开或关闭FreeRTOS的API



## 演示

1. cd code/s026/

2. make clean

3. make

4. make qemu

5. 退出qemu: 

   **Ctrl** + **a** + **x**



## 主页

更多信息，请关注B站 **芯片人阿伟**：

https://space.bilibili.com/243180540



