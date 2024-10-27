# Show me the code --- S037.FreeRTOS(10) corePKCS11

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



## corePKCS11

> PKCS#11: Public-Key Cryptography Standards #11
>
> https://docs.oasis-open.org/pkcs11/pkcs11-base/v3.0/pkcs11-base-v3.0.html

corePKCS11 是基于软件的 PKCS #11 应用程序编程接口 (API) 子集的模拟实现 。在生产设备转向特定安全硬件实现之前， 该系统可实现独立于硬件的快速原型设计和开发。

github：https://github.com/FreeRTOS/corePKCS11

通过 corePKCS11 可以完成以下操作：

1. 随机数生成
2. 哈希计算
3. 数据加密、解密
4. 数字签名、验签
5. HTTPS socket



## mbedTLS

github：https://github.com/Mbed-TLS/mbedtls

corePKCS11 的底层实现是基于 mbedTLS的，因此需要移植mbedTLS

本实例中使用的版本为：mbedtls-3.6.2.tar.bz2



## 演示

1. cd code/s037/

2. ./build.sh qemu

   退出qemu: **Ctrl** + **a** + **x**

3. ./build.sh clean



## 主页

更多信息，请关注B站 **芯片人阿伟**：

https://space.bilibili.com/243180540



