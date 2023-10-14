# Show me the code --- S006.CFI Nor Flash

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



## CFI Flash

CFI NOR Flash 是一种并行总线的Flash设备，接口类似于RAM，可直接读取内部数据。

在 qemu-riscv-virt 平台上，CFI Flash 地址为：`[VIRT_FLASH] =        { 0x20000000,     0x4000000 },`

它的常用 CMD 如下：

| 命令 | 功能                |
| ---- | ------------------- |
| 0x20 | block erase         |
| 0x50 | clear status        |
| 0x70 | read status         |
| 0x98 | query cfi           |
| 0xD0 | confirm cmd         |
| 0xE8 | block write         |
| 0xFF | reset to array mode |

通过封装这些 cfi cmd，就能实现对 cfi nor flash 的读写了。接口如下：

int CfiFlashInit(uint32_t pdrv, uint32_t priv);

int CfiFlashQuery(uint32_t pdrv);

int32_t CfiFlashRead(uint32_t pdrv, uint32_t *buffer, uint32_t offset, uint32_t nbytes);

int32_t CfiFlashWrite(uint32_t pdrv, const uint32_t *buffer, uint32_t offset, uint32_t nbytes);

int32_t CfiFlashErase(uint32_t pdrv, uint32_t offset);



## 演示

1. cd code/s006/

2. make clean

3. make

4. make qemu

5. 退出qemu: 

   **Ctrl** + **a** + **x**



## 主页

更多信息，请关注B站 **芯片人阿伟**：

https://space.bilibili.com/243180540



