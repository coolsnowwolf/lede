欢迎来到 Jay 的 OpenWrt 源码仓库

如何编译自己需要的 OpenWrt 固件 [How to build your OpenWrt firmware]

注意
不要用 root 用户进行编译
国内用户编译前最好准备好梯子
默认登陆IP 192.168.1.133 密码 password
编译命令
首先装好 Linux 系统，推荐 Debian 11 或 Ubuntu LTS

安装编译依赖

sudo apt update -y
sudo apt full-upgrade -y
sudo apt install -y ack antlr3 aria2 asciidoc autoconf automake autopoint binutils bison build-essential \
bzip2 ccache cmake cpio curl device-tree-compiler fastjar flex gawk gettext gcc-multilib g++-multilib \
git gperf haveged help2man intltool libc6-dev-i386 libelf-dev libglib2.0-dev libgmp3-dev libltdl-dev \
libmpc-dev libmpfr-dev libncurses5-dev libncursesw5-dev libreadline-dev libssl-dev libtool lrzsz \
mkisofs msmtp nano ninja-build p7zip p7zip-full patch pkgconf python2.7 python3 python3-pip libpython3-dev qemu-utils \
rsync scons squashfs-tools subversion swig texinfo uglifyjs upx-ucl unzip vim wget xmlto xxd zlib1g-dev
下载源代码，更新 feeds 并选择配置

git clone https://github.com/coolsnowwolf/lede
cd lede
./scripts/feeds update -a
./scripts/feeds install -a
make menuconfig
下载 dl 库，编译固件 （-j 后面是线程数，第一次编译推荐用单线程）

make download -j8
make V=s -j1
本套代码保证肯定可以编译成功。里面包括了 R23 所有源代码，包括 IPK 的。

你可以自由使用，但源码编译二次发布请注明我的 GitHub 仓库链接。谢谢合作！

二次编译：

cd lede
git pull
./scripts/feeds update -a
./scripts/feeds install -a
make defconfig
make download -j8
make V=s -j$(nproc)
如果需要重新配置：

rm -rf ./tmp && rm -rf .config
make menuconfig
make V=s -j$(nproc)
编译完成后输出路径：bin/targets
