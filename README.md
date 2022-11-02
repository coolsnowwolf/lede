默认登陆IP 192.168.1.1 密码 password

   sudo apt update -y
   
   sudo apt full-upgrade -y
   
   sudo apt install -y ack antlr3 aria2 asciidoc autoconf automake autopoint binutils bison build-essential \
   bzip2 ccache cmake cpio curl device-tree-compiler fastjar flex gawk gettext gcc-multilib g++-multilib \
   git gperf haveged help2man intltool libc6-dev-i386 libelf-dev libglib2.0-dev libgmp3-dev libltdl-dev \
   libmpc-dev libmpfr-dev libncurses5-dev libncursesw5-dev libreadline-dev libssl-dev libtool lrzsz \
   mkisofs msmtp nano ninja-build p7zip p7zip-full patch pkgconf python2.7 python3 python3-pip libpython3-dev qemu-utils \
   rsync scons squashfs-tools subversion swig texinfo uglifyjs upx-ucl unzip vim wget xmlto xxd zlib1g-dev
   
   git clone https://github.com/viviczh1/lede
   
   cd lede
   
   ./scripts/feeds update -a
   
   ./scripts/feeds install -a
   
   make menuconfig

   make download -j8
   
   make V=s -j1

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

