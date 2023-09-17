# setting build environment for risc-v

$env:GCC_DIR="D:\apps\xpack-riscv-none-elf-gcc-12.3.0-1\bin"
Write-Host "GCC_DIR=$env:GCC_DIR"

$env:MAKE_DIR="D:\apps\GNU_MCU_Eclipse\Build_Tools\2.12-20190422-1053\bin"
Write-Host "MAKE_DIR=$env:MAKE_DIR"

$env:QEMU_DIR="D:\apps\xpack-qemu-riscv-7.2.5-1\bin"
Write-Host "QEMU_DIR=$env:QEMU_DIR"

$env:TL_PATH="$env:GCC_DIR;$env:MAKE_DIR;$env:QEMU_DIR"
Write-Host "TL_PATH=$env:TL_PATH"

$env:PATH="C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;C:\Windows\System32\WindowsPowerShell\v1.0\"
$env:PATH="$env:TL_PATH;$env:PATH"
Write-Host "PATH=$env:PATH"

busybox bash
#powershell