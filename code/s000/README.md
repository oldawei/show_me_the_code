# S000 - 开发环境搭建

## 实验平台

实验是基于 **qemu risc-v virt platform** 的，这是一个基于 qemu 的 risc-v 虚拟计算机平台。不同于其他 qemu 计算机的是，该平台是完全虚拟的，现实中没有对应的计算机平台。推出该平台的目的，或者说作用，主要是为开发者提供一个统一的开发测试环境。

关于该平台的详细情况，可参阅官方文档：https://www.qemu.org/docs/master/system/riscv/virt.html



## 开发环境

**操作系统**: win10

**终端**：Windows Terminal，版本: >= 1.17.11461.0

https://github.com/microsoft/terminal/releases

**编译器**：xpack-riscv-none-elf-gcc-12.3.0-1

https://github.com/xpack-dev-tools/riscv-none-elf-gcc-xpack/releases/download/v12.3.0-1/xpack-riscv-none-elf-gcc-12.3.0-1-win32-x64.zip

**仿真器**：xpack-qemu-riscv-7.2.5-1

https://github.com/xpack-dev-tools/qemu-riscv-xpack/releases/download/v7.2.5-1/xpack-qemu-riscv-7.2.5-1-win32-x64.zip

**Make工具**：gnu-mcu-eclipse-windows-build-tools-2.12

https://github.com/xpack-dev-tools/windows-build-tools-xpack/releases/download/v2.12-20190422/gnu-mcu-eclipse-windows-build-tools-2.12-20190422-1053-win64.zip



## 演示

现在演示下搭建好的开发环境！

- 开发工具都安装到：D:\apps
- 代码 clone 到：D:\gitee\show_me_the_code
- 打开 dev_env.cc
- make -v

​		GNU Make 4.2.1

- riscv-none-elf-gcc -v

  gcc version 12.3.0 (xPack GNU RISC-V Embedded GCC x86_64)

- qemu-system-riscv32 --version

  xPack QEMU emulator version 7.2.5 (v7.2.5-xpack)

  

