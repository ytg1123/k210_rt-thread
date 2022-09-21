RT-Thread RTOS can support many architectures:<br>
SOC: cannan k210 <br>
Board BSP : SiPEED MAiX DOCK
apache_2.0 License<br>
AUTH: ytg1123@gmail.com YOSHITO TAGASHIRA 

■ 社内githubサイト情報 
<pre>
http://157.7.129.177:11080/taga/k210_rtthread_MAiXDOCK_bin
</pre>

■  git 関連　操作情報 <br>
<pre>
git config --global user.name "tagashira"
git config --global user.email "tagashira@zuiki.co.jp"
</pre>

<pre>
Create a new repository
git clone git@gitlab.vividnode.com:taga/k210_rtthread_MAiXDOCK_bin.git
cd k210_rtthread_MAiXDOCK_bin
touch README.md
git add README.md
git commit -m "add README"
git push -u origin master
</pre>

<pre>
Existing folder
cd existing_folder
git init
git remote add origin git@gitlab.vividnode.com:taga/k210_rtthread_MAiXDOCK_bin.git
git add .
git commit -m "Initial commit"
git push -u origin master
</pre>

<pre>
Existing Git repository
cd existing_repo
git remote rename origin old-origin
git remote add origin git@gitlab.vividnode.com:taga/k210_rtthread_MAiXDOCK_bin.git
git push -u origin --all
git push -u origin --tags
</pre>


■  k210 rt-thread実行コード <br>
$ cd $HOME<br>
$ cd k210_rtthread_MAiXDOCK_bin<br>
$ git clone git@github.com:ytg1123/k210_rtthread_MAiXDOCK_bin.git
$ cd k210_rtthread_MAiXDOCK_bin<br>

■  SiPEEDk210 SDCARD init and Settings <br>
$ cd $HOME<br>
$ cd k210_rtthread_MAiXDOCK_bin<br>

sdcard folderの中身をsdcardに複製コピーする<br>
$ cp -a sdcard/* /media/sda3/<br>

※　以下ファイル(file for : MikuMikudance file, kpu3 kmodel file,etc)<br>
20class.kmodel  <br>
7.wav  <br>
Pipeline.wav  <br>
hirose.wav  <br>
miku_xx_body.bmp  <br>
model.kmodel  <br>
motion.vmd        <br>
music1.wav   <br>
yolo100.kmodel  <br>
yolo75.kmodel <br>
6.wav           <br>
8.wav  <br>
boot.py       <br>
main.py     <br>
miku_xx_head.bmp  <br>
model.pmd     <br>
motionreadme.txt  <br>
yolo.kmodel  <br>
yolo50.kmodel <br>

■  K210 RtThread FlashROM書き込み手順 sipeed 公式kflashツール<br>
※ 以下より1.6.5より古いツールではうまく書き込みが出来なかった<br>

download path:　OS別に各種選択可能です<br>
[[https://github.com/sipeed/kflash_gui/releases/tag/v1.6.5]]<br>

download path:windows版<br>
[[https://github.com/sipeed/kflash_gui/releases/download/v1.6.5/kflash_gui_v1.6.5_2_windows.7z]]<br>

■ K210 RtThread FlashROM書き込み手順 sipeed 公式kflashツール<br>

※ これはWindowsアプリである<br>
※ 以下より1.6.5をdownloadする（これより古いツールではうまく書き込みが出来ない）<br>
※ download path:windows版<br>
[[https://github.com/sipeed/kflash_gui/releases/download/v1.6.5/kflash_gui_v1.6.5_2_windows.7z]]<br>

■ kflashツール<br>
※ Board Settings項タブでターゲットに合わせ ”sipeed MAix DOCK” を選択する<br>
※ selectfileでビルドしたファイルを選ぶ rtthread.bin<br>

select file: rtthread.maixdock.bin <br>
Board Settings: sipeed MAix DOCK<br>
Burn To: flash<br>
Serial Settings: COM5<br>
BaudRate: 1500000<br>
SpeedMode: slow mode<br>


■  動作手順 <br>
1.Windows Driver USB Serial CH340 install340 install<br>
2.SiPEED MAiXDOCK USB-port connect to Windows Teraterm Serial Console<br>
3. sdcard insert SiPEED MAiX DOCK board

 \ | /<br>
- RT -     Thread Operating System<br>
 / | \     4.0.3 build Sep 17 2021<br>
 2006 - 2020 Copyright by rt-thread team<br>
set spi1 freq 400198<br>
set spi1 freq 10075000<br>
Mount "/dev/sd0" on "/"<br>

msh /><br>
...<br>
fft_lcd          - start fft_lcd thread<br>
kpu_v3           - start kpu_v3 thread<br>
k210_mmd         - start k210_mmd thread<br>
...<br>


