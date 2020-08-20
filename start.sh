#!/bin/bash
#此脚本仅用于Ubuntu
sudo apt-get -y install build-essential asciidoc binutils bzip2 gawk gettext git libncurses5-dev libz-dev patch python3 python2.7 unzip zlib1g-dev lib32gcc1 libc6-dev-i386 subversion flex uglifyjs git-core gcc-multilib p7zip p7zip-full msmtp libssl-dev texinfo libglib2.0-dev xmlto qemu-utils upx libelf-dev autoconf automake libtool autopoint device-tree-compiler g++-multilib antlr3 gperf wget swig rsync
echo "创建用户：lede"
echo "并设置密码"
sudo adduser lede
cd /home/lede
#github加速
git clone https://hub.fastgit.org/chenaidairong/lede.git
cd lede
./scripts/feeds update -a
./scripts/feeds install -a
echo "进入用户的命令为 su lede"
echo "请选择自己的固件"
echo "环境已经配置好了，请按照文档操作"
echo "以次输入"
echo "选择硬件"
echo "make menuconfig"
echo "下载dl库"
echo "make -j8 download V=s"
echo "开始编译"
echo "make -j1 V=s"
su lede
