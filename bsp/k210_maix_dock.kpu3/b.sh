#!/bin/bash

[ -e rtthread.bin ] && rm rtthread.bin
[ -e rtthread.elf ] && rm rtthread.elf
[ -e rtthread.hed ] && rm rtthread.hed
[ -e rtthread.map ] && rm rtthread.map
[ -e build ] && rm -rf build/*

#toolchain="kendryte"
#toolchain="k210"
toolchain="gnu8.2.0-2.2-20190521"
#toolchain="gnu8.2.0-2.2-20210118"

#EXEC_PATH   = r'/opt/gnu-mcu-eclipse/riscv-none-gcc/8.2.0-2.2-20190521-0004/bin'
#EXEC_PATH   = r'/opt/gnu-mcu-eclipse/riscv-none-gcc/8.2.0-2.2-20210118-0222/bin'


if [ $toolchain = "kendryte" ]; then
	export LD_LIBRARY_PATH=/opt/kendryte-toolchain/bin:$LD_LIBRARY_PATH
	export RTT_EXEC_PATH=/opt/kendryte-toolchain/bin
elif [ $toolchain = "k210" ]; then
	export LD_LIBRARY_PATH=/opt/k210/8.3.0-1.1/bin:$LD_LIBRARY_PATH
	export RTT_EXEC_PATH=/opt/k210/8.3.0-1.1/bin
elif [ $toolchain = "gnu8.2.0-2.2-20190521" ]; then
	export LD_LIBRARY_PATH=/opt/gnu-mcu-eclipse/riscv-none-gcc/8.2.0-2.2-20190521-0004/bin:$LD_LIBRARY_PATH
	export RTT_EXEC_PATH=/opt/gnu-mcu-eclipse/riscv-none-gcc/8.2.0-2.2-20190521-0004/bin
elif [ $toolchain = "gnu8.2.0-2.2-20210118" ]; then
	export LD_LIBRARY_PATH=/opt/gnu-mcu-eclipse/riscv-none-gcc/8.2.0-2.2-20210118-0222/bin:$LD_LIBRARY_PATH
	export RTT_EXEC_PATH=/opt/gnu-mcu-eclipse/riscv-none-gcc/8.2.0-2.2-20210118-0222/bin
fi

#export LD_LIBRARY_PATH=/opt/kendryte-toolchain/bin:$LD_LIBRARY_PATH
#export RTT_EXEC_PATH=/opt/kendryte-toolchain/bin
#export LD_LIBRARY_PATH=/opt/k210/8.3.0-1.1/bin:$LD_LIBRARY_PATH
#export RTT_EXEC_PATH=/opt/k210/8.3.0-1.1/bin
#export RTT_EXEC_PATH=/home/taga/rt-thread/gcc-arm-none-eabi-6-2017-q2-update/bin
#export RTT_EXEC_PATH=/home/taga/rt-thread/gcc-arm-none-eabi-7-2018-q2-update/bin

scons --menuconfig
source ~/.env/env.sh
pkgs --update
scons -c

