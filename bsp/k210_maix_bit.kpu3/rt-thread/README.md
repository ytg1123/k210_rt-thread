# RT-Thread #

[中文页](README_zh.md) |

[![GitHub](https://img.shields.io/github/license/RT-Thread/rt-thread.svg)](https://github.com/RT-Thread/rt-thread/blob/master/LICENSE)
[![GitHub release](https://img.shields.io/github/release/RT-Thread/rt-thread.svg)](https://github.com/RT-Thread/rt-thread/releases)
[![Build Status](https://travis-ci.org/RT-Thread/rt-thread.svg)](https://travis-ci.org/RT-Thread/rt-thread)
[![Gitter](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/RT-Thread/rt-thread?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![GitHub pull-requests](https://img.shields.io/github/issues-pr/RT-Thread/rt-thread.svg)](https://github.com/RT-Thread/rt-thread/pulls)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat)](https://github.com/RT-Thread/rt-thread/pulls)

RT-Thread is an open source IoT operating system from China, which has strong scalability: from a tiny kernel running on a tiny core, for example ARM Cortex-M0, or Cortex-M3/4/7, to a rich feature system running on MIPS32, ARM Cortex-A8, ARM Cortex-A9 DualCore etc.

## Overview ##

RT-Thread RTOS like a traditional real-time operating system. The kernel has real-time multi-task scheduling, semaphore, mutex, mail box, message queue, signal etc. However, it has three different things:

* Device Driver;
* Component;
* Dynamic Module

The device driver is more like a driver framework, UART, IIC, SPI, SDIO, USB device/host, EMAC, MTD NAND etc. The developer can easily add low level driver and board configuration, then combined with the upper framework, he/she can use lots of features.

The Component is a software concept upon RT-Thread kernel, for example a shell (finsh/msh shell), virtual file system (FAT, YAFFS, UFFS, ROM/RAM file system etc), TCP/IP protocol stack (lwIP), POSIX (thread) interface etc. One component must be a directory under RT-Thread/Components and one component can be descripted by a SConscript file (then be compiled and linked into the system).

The Dynamic Module, formerly named as User Applicaion (UA) is a dynamic loaded module or library, it can be compiled standalone without Kernel. Each Dynamic Module has its own object list to manage thread/semaphore/kernel object which was created or initialized inside this UA. More information about UA, please visit another [git repo](https://github.com/RT-Thread/rtthread-apps).

## Board Support Package ##

RT-Thread RTOS can support many architectures:

* ARM Cortex-M0
* ARM Cortex-M3/M4/7
* ARM Cortex-R4
* ARM Cortex-A8/A9
* ARM920T/ARM926 etc
* MIPS32
* x86
* Andes
* C-Sky
* RISC-V
* PowerPC

## License ##

RT-Thread is Open Source software under the Apache License 2.0 since RT-Thread v3.1.1. License and copyright information can be found within the code.

    /*
     * Copyright (c) 2006-2018, RT-Thread Development Team
     *
     * SPDX-License-Identifier: Apache-2.0
     */

Since 9th of September 2018, PRs submitted by the community may be merged into the main line only after signing the Contributor License Agreement(CLA).

## Usage ##

RT-Thread RTOS uses [scons](http://www.scons.org) as building system. Therefore, please install scons and Python 2.7 firstly. 
So far, the RT-Thread scons building system support the command line compile or generate some IDE's project. There are some option varaibles in the scons building script (rtconfig.py):

* ```CROSS_TOOL``` the compiler which you want to use, gcc/keil/iar. 
* ```EXEC_PATH``` the path of compiler. 

In SConstruct file:

```RTT_ROOT``` This variable is the root directory of RT-Thread RTOS. If you build the porting in the bsp directory, you can use the default setting. Also, you can set the root directory in ```RTT_ROOT``` environment variable and not modify SConstruct files.

When you set these variables correctly, you can use command:

    scons

under BSP directory to simplely compile RT-Thread RTOS.

If you want to generate the IDE's project file, you can use command:

    scons --target=mdk/mdk4/mdk5/iar/cb -s

to generate the project file.

## Setting Up for build environment  ##

■rt-threadソース入手手順
$ cd
$ git clone https://github.com/RT-Thread/rt-thread.git
$ cd rt-thread

■rt-threadビルド用 pyenv環境構築
※複数のpythonバージョンを切り替える為に実施

$ git clone git://github.com/yyuu/pyenv.git ~/.pyenv
$ sudo apt install linuxbrew-wrapper
$ echo 'export PYENV_ROOT="$HOME/.pyenv"' >> ~/.bash_profile
$ echo 'export PATH="$PYENV_ROOT/bin:$PATH"' >> ~/.bash_profile
$ echo 'eval "$(pyenv init -)"' >> ~/.bash_profile
$ pyenv install 2.7.6
$ pyenv install 3.6.7
$ pyenv local 2.7.6
$ pyenv versions

■rt-threadビルド用 sconsの導入
$ sudo apt-get install scons

■rt-threadビルド用gcc導入 arm-none-eabi-gcc version 6.3.1 20170620
$ wget https://developer.arm.com/-/media/Files/downloads/gnu-rm/6-2017q2/gcc-arm-none-eabi-6-2017-q2-update-linux.tar.bz2
$ tar xvf gcc-arm-none-eabi-6-2017-q2-update-linux.tar.bz2
$ export RTT_EXEC_PATH=/home/taga/rt-thread/gcc-arm-none-eabi-6-2017-q2-update/bin:${PATH}

■rt-thread ビルド手順
※menuconfigで追加したいpackageの選択などをメニュ上で実施し.configを生成する
※パッケージとしてadbd等も選択できる

$ cd rt-thread/bsp/allwinner_tina
$ scons --menuconfig
$ source ~/.env/env.sh
$ scons -c
$ scons

■flash書き込みTool（sunxi-tools） build&install
$ sudo apt-get install zlib1g-dev
$ sudo apt-get install libusb-1.0-0-dev
$ git clone https://github.com/Icenowy/sunxi-tools.git
$ cd sunxi-tools
$ git checkout -b f1c100s origin/f1c100s
$ make
$ sudo make install

■splローダのビルド　tina-spl build
$ git clone https://github.com/uestczyh222/tina-spl.git
$ cd tina-spl
$ make
$ cp output/f1c100s.bin ../tina-spl.bin

■flash書き込み手順　rt-thread flash
$ sudo sunxi-fel -p write 0x00000000 tina-spl.bin
$ sudo sunxi-fel exec 0x00000000
$ sudo sunxi-fel -p write 0x80000000 rtthread.bin
$ sudo sunxi-fel exec 0x80000000

■JTAG  コネクタ（µSD breakout showing 14pin JTAG connector）
https://linux-sunxi.org/JTAG

■J-Link  コネクタ
以下PDFのPage9に記載されている
https://www.embitek.co.jp/technote/jlink/EUM_JP_JLink-Adapter.pdf

■J-Link Software v6.62d download
最新はv6.64だがdownloadに失敗するようなので使わない
https://www.segger.com/downloads/jlink#J-LinkSoftwareAndDocumentationPack
J-Link Software and Documentation pack for Windows

Z7213の場合のCPU設定：CPUはCORTEX-A7とする

■J-Link Software v6.64 install to windows PC
JLink_Windows_V662d.exe　これをinstallする。installはデスクトップにLinkを貼るにcheckをいれる、
その他はデフォルトのままでよい
USB/driverは自動的にinstされるようだ

installパス（ここにinstallされる）
C:\Program Files (x86)\SEGGER\JLink

Z7213の場合のCPU設定：CPUはCORTEX-A7とする


■J-Link Software v6.63e install to Ubuntu PC
v6.63e for linux as below $homeの下にフォルダを作って解凍
https://www.segger.com/downloads/jlink/JLink_Linux_x86_64_beta.tgz
$ cd ~/JLink_Linux_V663e_x86_64/

USB関連：以下ファイルを作成
$ touch /etc/udev/rules.d/45-jlink.rules 
$ vi /etc/udev/rules.d/45-jlink.rules
SUBSYSTEM=="usb", ATTR{idVendor}=="1366", ATTR{idProduct}=="0101", MODE="0666", GROUP="plugdev"

• Install USB Drivers(再起動後に有効となる)
$ sudo apt-get install libusb-1.0-0
$ sudo apt-get install libudev-dev
$ sudo apt-get install libboost-all-dev

再起動実施
$ sudo　　reboot

$ cd ~/JLink_Linux_V663e_x86_64/
$ ./JLinkGDBServer



■gdbサーバ起動　windows PC
デスクトップのLinkを実行（J-Link GDB Server V6.62d）
localhost-Onlyというチェックボックスを外す（外部gdbクライアントから接続する為に必須）

■gdbクライアント導入　windows PC
WindowsPC用GDBを使う場合はgdb.orgでソースを落とし自分でmakeしてください（cygwinとかmingwなど環境が必要です）
結構めんどくさいのでGDBクライアントはUbuntuで動かした方がよい。

■arm版gdbクライアント導入と起動方法　ubuntuPC
gdb クライアントをinstallする
$ sudo apt install gdb-arm-none-eabi
$ arm-none-eabi-gdb　--version
GNU gdb (7.10-1ubuntu3+9) 7.10
Copyright (C) 2015 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.  Type "show copying"
and "show warranty" for details.
This GDB was configured as "--host=x86_64-linux-gnu --target=arm-none-eabi".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<http://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
<http://www.gnu.org/software/gdb/documentation/>.
For help, type "help".
Type "apropos word" to search for commands related to "word".

$ arm-none-eabi-gdb
(gdb) 

※サーバとclientが同じubuntu　PC上ならば
(gdb) target remote localhost:2331
Remote debugging using localhost:2331
0xc001a048 in ?? ()
(gdb) 

※gdbサーバが別PCで動作している場合（同じセグメント内192.168.1.50でサーバが動作している場合）
(gdb) target remote 192.168.1.50:2331
Remote debugging using 192.168.1.50:2331
0xc00149a0 in ?? ()

(gdb) x/10i 0xffff0000
   0xffff0000:	svc	0x009f0000
   0xffff0004:	b	0xffff0380
   0xffff0008:	ldr	pc, [pc, #1040]	; 0xffff0420
   0xffff000c:	b	0xffff0300
   0xffff0010:	b	0xffff0280
   0xffff0014:	b	0xffff0404
   0xffff0018:	b	0xffff0200
   0xffff001c:	b	0xffff0400
   0xffff0020:	andeq	r0, r0, r0
   0xffff0024:	andeq	r0, r0, r0

(gdb) i r
r0             0xf	15
r1             0x0	0
r2             0xc0680248	3228041800
r3             0x0	0
r4             0xc0664000	3227926528
r5             0xc04ef9dc	3226401244
r6             0xc067fc40	3228040256
r7             0xffffffff	4294967295
r8             0xc0a63c40	3232119872
r9             0xc065cae4	3227896548
r10            0x0	0
r11            0x0	0
r12            0x0	0
sp             0xc0665fa8	0xc0665fa8
lr             0xc00106f8	3221292792
pc             0xc001a048	0xc001a048
cpsr           0x600d0093	1611464851

sudo sunxi-fel -v version
sudo sunxi-fel -v write-with-progress 0x2000 fes1.fex
sudo sunxi-fel -v exec 0x2000
sudo sunxi-fel  write-with-progress 0x4a000000 rtthread.bin
sudo sunxi-fel -v exec 0x4a000000


## build shell scripts examples as bellow ##

    --- bld.sh ---->
    #!/bin/bash
    set -e
    export RTT_EXEC_PATH=/home/taga/rt-thread/gcc-arm-none-eabi-6-2017-q2-update/bin:${PATH}
    pushd .
    cd bsp/bsp_a33
    scons --menuconfig
    source ~/.env/env.sh
    scons -c
    scons
    popd
    <--- bld.sh ----


NOTE: RT-Thread scons building system will tailor the system according to your rtconfig.h configuration header file. For example, if you disable the lwIP in the rtconfig.h by commenting the ```#define RT_USING_LWIP```, the generated project file should have no lwIP related files.





## Contribution ##

Please refer the contributors in the github. Thank all of RT-Thread Developers.
