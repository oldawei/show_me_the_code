# Show me the code --- S005.goldfish rtc

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



## goldfish rtc

Google Goldfish RTC 是 qemu 中的虚拟 RTC 硬件设备，用于模拟 RTC 功能。

详细技术文档可参考：

docs/docs_GOLDFISH-VIRTUAL-HARDWARE.TXT.pdf

https://android.googlesource.com/platform/external/qemu/+/master/docs/GOLDFISH-VIRTUAL-HARDWARE.TXT

它在 qemu-riscv-virt 平台上的地址为：`[VIRT_RTC] =          {   0x101000,        0x1000 },`



## 寄存器

Goldfish RTC 的寄存器很简单，只有几个：

| 地址 | 名称            | 描述                          |
| ---- | --------------- | ----------------------------- |
| 0x00 | TIME_LOW        | low bits of current time      |
| 0x04 | TIME_HIGH       | high bits of current time     |
| 0x08 | ALARM_LOW       | low bits of alarm time        |
| 0x0c | ALARM_HIGH      | high bits of alarm time       |
| 0x10 | IRQ_ENABLED     | enable alarm interrupt        |
| 0x14 | CLEAR_ALARM     | disarm an existing alarm      |
| 0x18 | ALARM_STATUS    | alarm status (running or not) |
| 0x1c | CLEAR_INTERRUPT | clear interrupt               |

 RTC 封装实现了如下接口：

void goldfish_rtc_init(u32 base_addr);

int goldfish_rtc_read_alarm(struct rtc_wkalrm *alrm);

int goldfish_rtc_set_alarm(struct rtc_wkalrm *alrm);

int goldfish_rtc_alarm_irq_enable(unsigned int enabled);

int goldfish_rtc_interrupt(void);

int goldfish_rtc_read_time(struct rtc_time *tm);

u64 goldfish_rtc_read_time_sec(void);

int goldfish_rtc_set_time(struct rtc_time *tm);



## 演示

1. cd code/s005/

2. make clean; make

3. make qemu

4. 退出qemu: 

   **Ctrl** + **a** + **x**

   


## 主页

更多信息，请关注B站 **芯片人阿伟**：

https://space.bilibili.com/243180540



