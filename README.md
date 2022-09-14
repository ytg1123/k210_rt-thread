h1. k210版Rt-Threadのビルド手順

!https://github.com/ytg1123/k210_rt-thread/M-14471.jpg!
<pre>
■ rt-thread git ソース入手先
※ PATH　https://github.com/ytg1123/k210_rt-thread  

$ cd $HOME
$ git clone https://github.com/ytg1123/k210_rt-thread.git 
$ cd k210_rt-thread

■ Kendryte-gccのdownloadと設定（Kendryte-gcc公開サイト版）
$ cd $HOME
$ mkdir -p /opt
$ cd /opt
$ wget https://s3.cn-north-1.amazonaws.com.cn/dl.kendryte.com/documents/kendryte-toolchain-ubuntu-amd64-8.2.0-20190213.tar.gz
$ tar xvf kendryte-toolchain-ubuntu-amd64-8.2.0-20190213.tar.gz
$ rm kendryte-toolchain-ubuntu-amd64-8.2.0-20190213.tar.gz
$ export LD_LIBRARY_PATH=/opt/kendryte-toolchain/bin

■ gnu gccコンパイラ
※ 以下にtar形式で圧縮済みのtarballを格納しているのでいずれかを使用してほしい
https://github.com/ilg-archived/riscv-none-gcc/releases/tag/v8.2.0-2.2-20190521
https://github.com/ilg-archived/riscv-none-gcc/releases/tag/v8.2.0-2.2-20190521

$ cd $HOME
$ cd k210-rt-thread

※ gnu-mcu-eclipse版
※ /opt/フォルダ直下に以下の通りに各gccを格納する。
$ tar xvzfp gnu-mcu-eclipse_riscv-none-gcc.tar.bz2　-C /opt

taga@taga-CFSZ5:~/k210-rt-thread$ ls /opt/gnu-mcu-eclipse/riscv-none-gcc/
8.2.0-2.2-20190521-0004  

■ rtthread/k210_maixのビルド手順
※ MAiX BIT向けです
$ cd $HOME
$ cd k210-rt-thread/bsp/k210_maix_bit.kpu3

※ MAiX M1DOCK向けです
$ cd $HOME
$ cd k210-rt-thread/bsp/k210_maix_dock.kpu3

■ MAiX M1DOCK向け

#!/bin/bash

[ -e rtthread.bin ] && rm rtthread.bin
[ -e rtthread.elf ] && rm rtthread.elf
[ -e rtthread.hed ] && rm rtthread.hed
[ -e rtthread.map ] && rm rtthread.map
[ -e build ] && rm -rf build/*

#toolchain="kendryte"
#toolchain="k210"
toolchain="gnu8.2.0-2.2-20190521"

#EXEC_PATH   = r'/opt/gnu-mcu-eclipse/riscv-none-gcc/8.2.0-2.2-20190521-0004/bin'


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

cp defconfig .config
scons --menuconfig
source ~/.env/env.sh
pkgs --update
scons -c
scons 2>&1 | tee out
cp rtthread.bin rtthread.maixdock.bin


■ MAiX BIT向け
カメラによってconfigが違います、lvglも含めてov5640版が最新です
defconfig_ov5640_lvgl
defconfig_ov2640

#!/bin/bash

[ -e rtthread.bin ] && rm rtthread.bin
[ -e rtthread.elf ] && rm rtthread.elf
[ -e rtthread.hed ] && rm rtthread.hed
[ -e rtthread.map ] && rm rtthread.map
[ -e build ] && rm -rf build/*

#toolchain="kendryte"
#toolchain="k210"
toolchain="gnu8.2.0-2.2-20190521"

#EXEC_PATH   = r'/opt/gnu-mcu-eclipse/riscv-none-gcc/8.2.0-2.2-20190521-0004/bin'


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

cp defconfig_ov5640_lvgl .config
scons --menuconfig
source ~/.env/env.sh
pkgs --update
scons -c
scons 2>&1 | tee out
cp rtthread.bin rtthread.maixbit.bin


■ scons --menuconfig の注意事項


※ ボードの\選択をすることMAIXM1DOCKかBITのいずれか
MAIX Board Selection for You Use (use MAIXM1DOCK)  --->

※ RT-Threadに組み込まれたMicroPythonは選択してはいけない
※ このソースではh/wデバイスが使えない事とモジュールが使えないので使う意味が無い

RT-Thread online packages  --->
　language packages  --->
　　[ ] MicroPython: A lean and efficient Python implementation for

※ kendryte k210 demo packageソースは選択してはいけない
※ kendryte k210 demo packageソースはkendryte_sdk環境で動作するものでありrtthreadでは動作しない

RT-Thread online packages  --->
　　　　miscellaneous packages  --->
 [ ] kendryte k210 demo package  ----


■ k210_maix_bit.kpu3のビルド 説明

$ cd $HOME
$ cd k210-rt-thread/bsp/k210_maix_bit.kpu3/

1)Kconfig
※ fft_lcd_MAiXはマイク入力をfft変換しそれをLCD表示するアプリ
※ 以下の様に"fft_lcd_MAiX/Kconfig"を追記する

source "fft_lcd_MAiX/Kconfig"
source "$RTT_DIR/Kconfig"
source "$PKGS_DIR/Kconfig"


2)fft_lcd_MAiXの中身
※  fft_lcd_MAiX/Kconfigでは以下の様にボード選択をする

menuconfig BSP_USING_MAIX
    choice
        prompt "MAIX Board Selection for You Use"
        default BSP_USING_MAIXBIT
        config BSP_USING_MAIXBIT
            bool "use MAIXBIT"
        config BSP_USING_MAIXM1DOCK
            bool "use MAIXM1DOCK"
    endchoice


3)fft_lcd_MAiX/SConscriptの中身
※ 以下の様にビルド指示を設定している

# RT-Thread building script for component

from building import *

cwd     = GetCurrentDir()
src = Glob('*.c')
CPPPATH = [cwd]

group = DefineGroup('fft_lcd_MAiX', src, depend = [''], CPPPATH = CPPPATH)

objs = [group]

list = os.listdir(cwd)

for item in list:
    if os.path.isfile(os.path.join(cwd, item, 'SConscript')):
        objs = objs + SConscript(os.path.join(item, 'SConscript'))

Return('objs')

4)board_config.h
※ LCDの種別設定等を設定してる

#ifndef _BOARD_CONFIG_
#define _BOARD_CONFIG_

#define  OV5640             0
#define  OV2640             1

#define  BOARD_KD233        0
#define  BOARD_LICHEEDAN    1
#define  BOARD_K61          0

#if (OV5640 && OV2640) || (!OV5640 && !OV2640)
#error ov sensor only choose one
#endif

#if (BOARD_LICHEEDAN && BOARD_KD233) || (BOARD_LICHEEDAN && BOARD_K61) || (BOARD_K61 && BOARD_KD233) || (!BOARD_LICHEEDAN && !BOARD_KD233 && !BOARD_K61)
#error board only choose one
#endif

#endif


■ fft_lcd_MAiX/main.cの説明
※ MAiXボードではBIT/DOCKでI2Sポート（マイク入力ポート）番号が違ってる
※ 以下の様にボードタイプで識別している

#ifdef BSP_USING_MAIXBIT
    fpioa_set_function(20, FUNC_I2S0_IN_D0);
    fpioa_set_function(19, FUNC_I2S0_WS);
    fpioa_set_function(18, FUNC_I2S0_SCLK);
#elf BSP_USING_MAIXM1DOCK
    fpioa_set_function(20, FUNC_I2S0_IN_D0);
    fpioa_set_function(30, FUNC_I2S0_WS);
    fpioa_set_function(32, FUNC_I2S0_SCLK);
#else
    fpioa_set_function(20, FUNC_I2S0_IN_D0);
    fpioa_set_function(30, FUNC_I2S0_WS);
    fpioa_set_function(32, FUNC_I2S0_SCLK);
#endif

※ mshコマンドとしてfft_lcdとfft_lcd_stopの２コマンドを実装している
※制限：このコマンドではfft_lcdでthread起動しているがfft_lcd_stopコマンドでmshに戻り切れていないので終了はリセット実行か電源リセット。

MSH_CMD_EXPORT(fft_lcd,start fft_lcd thread);
MSH_CMD_EXPORT(fft_lcd_stop,stop fft_lcd thread);

※ 起動時、自動起動のコマンドとして実行させるには以下を追加する

INIT_APP_EXPORT(fft_lcd);  

■applicationsフォルダの説明
※ ここにはcamera_testコマンドを実装している、camera_testコマンド実行でcamera撮影でーたをLCDに随時表示する
※cam_stopでコマンドが停止する

※起動時、自動起動のコマンドとして実行させるには以下を追加する

INIT_APP_EXPORT(camera_test);  


## 7. 参考

  [[https://www.rt-thread.org/page/download.html]]
  [[https://s3.cn-north-1.amazonaws.com.cn/dl.kendryte.com/documents/kendryte_datasheet_20180919020633.pdf]]

</pre>
