# Welcome to Lean's git source of OpenWrt and packages

I18N: [English](README_EN.md) | [简体中文](README.md)

## Official Channels
<!--
如有技术问题需要讨论或者交流，欢迎加入以下群：
1. QQ 讨论群： Op固件技术研究群 ,号码 891659613 ，加群链接：[点击加入](https://jq.qq.com/?_wv=1027&k=XL8SK5aC "Op固件技术研究群")
2. TG 讨论群： OP 编译官方大群 ，加群链接：[点击加入](https://t.me/JhKgAA6Hx1 "OP 编译官方大群")
3. Rockchip RK3568 预编译固件发布 Release 下载更新地址 (包括 H68K )：<https://github.com/coolsnowwolf/lede/releases/tag/20220716>
-->

If you have technical questions for discussion or sharing, feel free to join the following channels:
1. QQ Group: *OpenWRT Firmware Technical Research Group*, Group Number is `891659613`. Join the group: [Link](https://jq.qq.com/?_wv=1027&k=XL8SK5aC "Op固件技术研究群").
    -  [Click to download QQ client](https://im.qq.com/pcqq).
2. Telegram Group: *OpenWRT Firmware Technical Research Group*. Join the group: [Link](https://t.me/JhKgAA6Hx1 "OP 编译官方大群").
3. Rockchip **RK3568** precompiled firmware release (including H68K): [Click to download](https://github.com/coolsnowwolf/lede/releases/tag/20220716).

<div align="left">
    <a href="https://item.taobao.com/item.htm?spm=a230r.1.14.11.4bb55247rdHEAP&id=702787603594&ns=1&abbucket=17#detail
">
        <img style="margin: 0px 0px 0px 40px;" src="https://github.com/coolsnowwolf/lede/blob/master/doc/h68k.jpg?raw=true" width=600  />
    </a>
</div>

4. Rockchip **RK3588** precompiled firmware release (including H68K): [Click to download](https://github.com/coolsnowwolf/lede/releases/tag/20230609).


## Notice
<!--
1. **不要用 root 用户进行编译**
2. 国内用户编译前最好准备好梯子
3. 默认登陆IP 192.168.1.1 密码 password
-->
1. **Never compile OpenWRT as `root`**
2. If you are living in mainland China, please make sure you could visit the **REAL** Internet.
3. Default login IP is `192.168.1.1`, password is `password`.

## How to Compile
<!--
1. 首先装好 Linux 系统，推荐 Debian 11 或 Ubuntu LTS
2. 安装编译依赖

   ```bash
   sudo apt update -y
   sudo apt full-upgrade -y
   sudo apt install -y ack antlr3 asciidoc autoconf automake autopoint binutils bison build-essential \
   bzip2 ccache cmake cpio curl device-tree-compiler fastjar flex gawk gettext gcc-multilib g++-multilib \
   git gperf haveged help2man intltool libc6-dev-i386 libelf-dev libglib2.0-dev libgmp3-dev libltdl-dev \
   libmpc-dev libmpfr-dev libncurses5-dev libncursesw5-dev libreadline-dev libssl-dev libtool lrzsz \
   mkisofs msmtp nano ninja-build p7zip p7zip-full patch pkgconf python2.7 python3 python3-pyelftools \
   libpython3-dev qemu-utils rsync scons squashfs-tools subversion swig texinfo uglifyjs upx-ucl unzip \
   vim wget xmlto xxd zlib1g-dev python3-setuptools
   ```
-->

1. Install a Linux distribution, Debian 11 or Ubuntu LTS is recommended.
2. Install dependencies:

   ```bash
   sudo apt update -y
   sudo apt full-upgrade -y
   sudo apt install -y ack antlr3 asciidoc autoconf automake autopoint binutils bison build-essential \
   bzip2 ccache cmake cpio curl device-tree-compiler fastjar flex gawk gettext gcc-multilib g++-multilib \
   git gperf haveged help2man intltool libc6-dev-i386 libelf-dev libglib2.0-dev libgmp3-dev libltdl-dev \
   libmpc-dev libmpfr-dev libncurses5-dev libncursesw5-dev libreadline-dev libssl-dev libtool lrzsz \
   mkisofs msmtp nano ninja-build p7zip p7zip-full patch pkgconf python2.7 python3 python3-pyelftools \
   libpython3-dev qemu-utils rsync scons squashfs-tools subversion swig texinfo uglifyjs upx-ucl unzip \
   vim wget xmlto xxd zlib1g-dev python3-setuptools
   ```

<!--
3. 下载源代码，更新 feeds 并选择配置

   ```bash
   git clone https://github.com/coolsnowwolf/lede
   cd lede
   ./scripts/feeds update -a
   ./scripts/feeds install -a
   make menuconfig
   ```

4. 下载 dl 库，编译固件
（-j 后面是线程数，第一次编译推荐用单线程）

   ```bash
   make download -j8
   make V=s -j1
   ```
-->

3. Clone the source code, update `feeds` and configure:

   ```bash
   git clone https://github.com/coolsnowwolf/lede
   cd lede
   ./scripts/feeds update -a
   ./scripts/feeds install -a
   make menuconfig
   ```

4. Download libraries and compile firmware
   > (`-j` is the thread count, single-thread is recommended for the first build):

   ```bash
   make -j8 download V=s
   make -j1 V=s
   ```

<!--
本套代码保证肯定可以编译成功。里面包括了 R23 所有源代码，包括 IPK 的。

你可以自由使用，但源码编译二次发布请注明我的 GitHub 仓库链接。谢谢合作！
-->

These commands are supposed to compile the source code successfully.
All source code of R23 is included, including IPK.

You can use this source code freely, but please link this GitHub repository when redistributing.
Thank you for your cooperation!

<!--
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
-->

Rebuild:

```bash
cd lede
git pull
./scripts/feeds update -a
./scripts/feeds install -a
make defconfig
make download -j8
make V=s -j$(nproc)
```

If reconfiguration is need:

```bash
rm -rf ./tmp && rm -rf .config
make menuconfig
make V=s -j$(nproc)
```

Build artifacts will be outputted to `bin/targets` directory.

### If you are using WSL/WSL2 as your build environment

<!--
由于 WSL 的 PATH 中包含带有空格的 Windows 路径，有可能会导致编译失败，请在 `make` 前面加上：

```bash
PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
```
-->
WSL's `PATH` potentially contain Windows paths with spaces, which may cause compilation failure. Please add the following lines to your local environment profiles before compiling:

```bash
# Update and reload your profile, ~/.bashrc for example.
cat << EOF >> ~/.bashrc
export PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:$PATH"
EOF
source ~/.bashrc
```

<!--
由于默认情况下，装载到 WSL 发行版的 NTFS 格式的驱动器将不区分大小写，因此大概率在 WSL/WSL2 的编译检查中会返回以下错误：

```txt
Build dependency: OpenWrt can only be built on a case-sensitive filesystem 
```

一个比较简洁的解决方法是，在 `git clone` 前先创建 Repository 目录，并为其启用大小写敏感：

```powershell
# 以管理员身份打开终端
PS > fsutil.exe file setCaseSensitiveInfo <your_local_lede_path> enable
# 将本项目 git clone 到开启了大小写敏感的目录 <your_local_lede_path> 中
PS > git clone git@github.com:coolsnowwolf/lede.git <your_local_lede_path>
```

> 对已经 `git clone` 完成的项目目录执行 `fsutil.exe` 命令无法生效，大小写敏感只对新增的文件变更有效。
-->

NTFS-formatted drives mounted to a WSL distribution will be case-insensitive by default. This will cause the following error when compiling in WSL/WSL2:

```txt
Build dependency: OpenWrt can only be built on a case-sensitive filesystem 
```

A simple solution is to create a case-sensitive directory for the repository before `git clone`:

```powershell
# Open a terminal as administrator
PS > fsutil.exe file setCaseSensitiveInfo <your_local_lede_path> enable
# Clone this repository to the case-sensitive directory <your_local_lede_path>
PS > git clone git@github.com:coolsnowwolf/lede.git <your_local_lede_path>
```

> For directories that have already been `git clone`d, `fsutil.exe` will not take effect. Case sensitivity will only be enabled for new changes in the directory.

### macOS Compilation
<!--
1. 在 AppStore 中安装 Xcode

2. 安装 Homebrew：

   ```bash
   /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
   ```

3. 使用 Homebrew 安装工具链、依赖与基础软件包:

   ```bash
   brew unlink awk
   brew install coreutils diffutils findutils gawk gnu-getopt gnu-tar grep make ncurses pkg-config wget quilt xz
   brew install gcc@11
   ```

4. 然后输入以下命令，添加到系统环境变量中：

   ```bash
   echo 'export PATH="/usr/local/opt/coreutils/libexec/gnubin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/usr/local/opt/findutils/libexec/gnubin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/usr/local/opt/gnu-getopt/bin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/usr/local/opt/gnu-tar/libexec/gnubin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/usr/local/opt/grep/libexec/gnubin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/usr/local/opt/gnu-sed/libexec/gnubin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/usr/local/opt/make/libexec/gnubin:$PATH"' >> ~/.bashrc
   ```

5. 重新加载一下 shell 启动文件 `source ~/.bashrc`，然后输入 `bash` 进入 bash shell，就可以和 Linux 一样正常编译了
-->

1. Install Xcode from AppStore
2. Install Homebrew:
   ```bash
   /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
   ```
3. Install toolchain, dependencies and packages with Homebrew:
   ```bash
   brew unlink awk
   brew install coreutils diffutils findutils gawk gnu-getopt gnu-tar grep make ncurses pkg-config wget quilt xz
   brew install gcc@11
   ```
4. Update your system environment:
   ```bash
   echo 'export PATH="/usr/local/opt/coreutils/libexec/gnubin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/usr/local/opt/findutils/libexec/gnubin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/usr/local/opt/gnu-getopt/bin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/usr/local/opt/gnu-tar/libexec/gnubin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/usr/local/opt/grep/libexec/gnubin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/usr/local/opt/gnu-sed/libexec/gnubin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/usr/local/opt/make/libexec/gnubin:$PATH"' >> ~/.bashrc
   ```
5. Reload your shell profile `source ~/.bashrc && bash`, then you can compile normally like Linux.

## Declaration
<!--
1. 源代码中绝不含任何后门和可以监控或者劫持你的 HTTPS 的闭源软件， SSL 安全是互联网最后的壁垒。安全干净才是固件应该做到的；

2. 想学习 OpenWrt 开发，但是摸不着门道？自学没毅力？基础太差？怕太难学不会？跟着佐大学 OpenWrt 开发入门培训班助你能学有所成
报名地址：[点击报名](http://forgotfun.org/2018/04/openwrt-training-2018.html "报名")

3. QCA IPQ60xx 开源仓库地址：<https://github.com/coolsnowwolf/openwrt-gl-ax1800>

4. 存档版本仓库地址：<https://github.com/coolsnowwolf/openwrt>
-->
1. This source code doesn't contain any backdoors or closed source applications that can monitor/capture your HTTPS traffic. SSL security is the final castle of cyber security. Safety is what a firmware should do.
2. Want to learn OpenWRT development but don't know how to start? Can't motivate yourself for self-learning? Do not have enough fundamental knowledge? Learn OpenWRT development with Mr. Zuo through his Beginner OpenWRT Training Course. Click [here](http://forgotfun.org/2018/04/openwrt-training-2018.html) to register.
3. QCA IPQ60xx open source repository: <https://github.com/coolsnowwolf/openwrt-gl-ax1800>
4. OpenWRT Archive repository: <https://github.com/coolsnowwolf/openwrt>

## Introduction to Software Routers
<!--
硬酷R2 - N95/N300迷你四网HomeLab服务器

(商品介绍页面 - 硬酷科技（支持花呗）)：

[预售链接](https://item.taobao.com/item.htm?ft=t&id=719159813003)

[![r1](doc/r1.jpg)](https://item.taobao.com/item.htm?ft=t&id=719159813003)
-->
Yingku R2 - N95/N300 Mini Four-Network HomeLab Server

(Introduction page - Yingku Technology (support AliPay Huabei)):

[Pre-sale link](https://item.taobao.com/item.htm?ft=t&id=719159813003)
<div align="left">
<a href="https://item.taobao.com/item.htm?ft=t&id=719159813003">
  <img src="doc/r1.jpg" width = "600" alt="" align=center />
</a>
</div>
<br>

## Donation

<!--
如果你觉得此项目对你有帮助，可以捐助我们，以鼓励项目能持续发展，更加完善

 ![star](doc/star.png)
-->
If this project did helped you, please consider donating to support the development of this project.

<div align="left">    
  <img src="./doc/star.png" width = "400" alt="" align=center />
</div>
<br>
