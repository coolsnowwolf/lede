默认登陆IP 192.168.1.3 密码 password 关闭dhcp，适合旁路网关，做主路由需要手动开启。
```bash
sudo apt update -y
sudo apt full-upgrade -y
sudo apt install -y git ack antlr3 aria2 asciidoc autoconf automake autopoint binutils bison build-essential \
bzip2 ccache cmake cpio curl device-tree-compiler fastjar flex gawk gettext gcc-multilib g++-multilib \
git gperf haveged help2man intltool libc6-dev-i386 libelf-dev libglib2.0-dev libgmp3-dev libltdl-dev \
libmpc-dev libmpfr-dev libncurses5-dev libncursesw5-dev libreadline-dev libssl-dev libtool lrzsz \
mkisofs msmtp nano ninja-build p7zip p7zip-full patch pkgconf python2.7 python3 python3-pip libpython3-dev qemu-utils \
rsync scons squashfs-tools subversion swig texinfo uglifyjs upx-ucl unzip vim wget xmlto xxd zlib1g-dev
```
```bash
git clone https://github.com/viviczh1/lede
cd lede
./scripts/feeds update -a
./scripts/feeds install -a
```
```bash
make menuconfig
make download -j8
make V=s -j1
 ```
二次编译：
```bash
cd lede
git pull
./scripts/feeds update -a
./scripts/feeds install -a
make defconfig
make download -j8
make V=s -j$(nproc)
```
如果需要重新配置：
```bash
rm -rf ./tmp && rm -rf .config
make menuconfig
make V=s -j$(nproc)
```
编译完成后输出路径：bin/targets

错误排查：

1、不要使用root用户编译，可以新建一个账户

2、遇到po2lmo错误可以对luci-base先编译，运行一次
```bash
make package/feeds/luci/luci-base/compile V=99
```
