#!/bin/bash
#此脚本仅用于Ubuntu
sudo apt-get -y install build-essential asciidoc binutils bzip2 gawk gettext git libncurses5-dev libz-dev patch python3 python2.7 unzip zlib1g-dev lib32gcc1 libc6-dev-i386 subversion flex uglifyjs git-core gcc-multilib p7zip p7zip-full msmtp libssl-dev texinfo libglib2.0-dev xmlto qemu-utils upx libelf-dev autoconf automake libtool autopoint device-tree-compiler g++-multilib antlr3 gperf wget swig rsync
echo "创建用户：tt"
sudo adduser tt
cd /home/tt
#github加速
git https://hub.fastgit.org/chenaidairong/lede.git
cd lede
./scripts/feeds update -a
./scripts/feeds install -a
echo "请选择自己的固件"
make menuconfig
make -j8 download V=s
make -j1 V=s
