欢迎来到zxmlysxl的Openwrt源码仓库！
本源码fork自lean，目的为自用，针对源码的修改不一定满足您的需求
=

如何编译自己需要的 OpenWrt 固件
-
注意：
-
1. **不要用root 用户进行编译！！！**
2. 国内用户编译前最好准备好梯子
3. 默认登陆IP 192.168.32.10 密码为空


编译命令如下:
-
1. 首先装好 Ubuntu 64bit，推荐 Ubuntu 20.04 LTS x64

2. 命令行输入 `sudo apt-get update` ，然后输入
   `
   sudo apt-get -y install ack antlr3 asciidoc autoconf automake autopoint binutils bison build-essential \
bzip2 ccache cmake cpio curl device-tree-compiler fastjar flex gawk gettext gcc-multilib g++-multilib \
git gperf haveged help2man intltool libc6-dev-i386 libelf-dev libglib2.0-dev libgmp3-dev libltdl-dev \
libmpc-dev libmpfr-dev libncurses5-dev libncursesw5-dev libreadline-dev libssl-dev libtool lrzsz \
mkisofs msmtp nano ninja-build p7zip p7zip-full patch pkgconf python2.7 python3 python3-pip qemu-utils \
rsync scons squashfs-tools subversion swig texinfo uglifyjs upx-ucl unzip vim wget xmlto xxd zlib1g-dev

2.1 如果你是安装了Ubuntu 21.10 LTS x64,则运行以下命令
  
   sudo apt-get -y install build-essential asciidoc binutils bzip2 gawk gettext git libncurses5-dev patch python3 python2.7 unzip zlib1g-dev lib32gcc-s1 libc6-dev-i386 subversion flex uglifyjs git gcc-multilib p7zip p7zip-full msmtp libssl-dev texinfo libglib2.0-dev xmlto qemu-utils upx-ucl libelf-dev autoconf automake libtool autopoint device-tree-compiler g++-multilib antlr3 gperf wget curl swig rsync ack bison ccache cmake cpio fastjar haveged help2man intltool libgmp3-dev libltdl-dev libmpc-dev libmpfr-dev libncursesw5-dev libreadline-dev lrzsz mkisofs nano ninja-build pkgconf python3-pip scons squashfs-tools vim xxd

3. 使用 `git clone https://github.com/zxmlysxl/lede` 命令下载好源代码，然后 `cd lede` 进入目录

4. ```bash
   ./scripts/feeds update -a
   ./scripts/feeds install -a
   make menuconfig
   ```

5. `make -j8 download V=s` 下载dl库（国内请尽量全局科学上网）

6. 输入 `make -j1 V=s` （-j1 后面是线程数。第一次编译推荐用单线程）即可开始编译你要的固件了。

本套代码保证肯定可以编译成功。里面包括了 R21 所有源代码，包括 IPK 的。

你可以自由使用，但源码编译二次发布请注明我的 GitHub 仓库链接。谢谢合作！
=
二次编译：

```bash
cd lede
git pull
./scripts/feeds update -a && ./scripts/feeds install -a
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

如果你使用WSL或WSL2进行编译：
------
由于wsl的PATH路径中包含带有空格的Windows路径，有可能会导致编译失败，请在将make -j1 V=s或make -j$(($(nproc) + 1)) V=s改为

首次编译：
```bash
PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin make -j1 V=s 
```
二次编译：
```bash
PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin make -j$(($(nproc) + 1)) V=s
```
------
macOS 原生系统进行编译：
------
1.在 AppStore 中安装 Xcode

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

4.然后输入以下命令，添加到系统环境变量中：
```bash
echo 'export PATH="/usr/local/opt/coreutils/libexec/gnubin:$PATH"' >> ~/.bashrc
echo 'export PATH="/usr/local/opt/findutils/libexec/gnubin:$PATH"' >> ~/.bashrc
echo 'export PATH="/usr/local/opt/gnu-getopt/bin:$PATH"' >> ~/.bashrc
echo 'export PATH="/usr/local/opt/gnu-tar/libexec/gnubin:$PATH"' >> ~/.bashrc
echo 'export PATH="/usr/local/opt/grep/libexec/gnubin:$PATH"' >> ~/.bashrc
echo 'export PATH="/usr/local/opt/gnu-sed/libexec/gnubin:$PATH"' >> ~/.bashrc
echo 'export PATH="/usr/local/opt/make/libexec/gnubin:$PATH"' >> ~/.bashrc
```

5.重新加载一下 shell 启动文件
```bash
source ~/.bashrc
```

然后输入 bash 命令，进入bash shell，就可以和 Linux 一样正常编译了

------

特别提示：
------
  源代码中绝不含任何后门和可以监控或者劫持你的 HTTPS 的闭源软件， SSL 安全是互联网最后的壁垒。安全干净才是固件应该做到的；
