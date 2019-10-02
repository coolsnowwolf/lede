如何编译自己需要的 OpenWrt 固件
====

**1. 不要Fork我这个repo，此repo不会实时更新，很可能缺少新功能或者留有未修复的BUG。**  
**2. 以下命令可以作为你搭建环境和编译的参考**

注意：
1. __不要__ 用 `root` 用户进行编译！！！
2. 国内用户编译前最好准备好梯子
3. 编译成功后，固件默认登陆IP 192.168.5.1, 用户名`root`，密码 `password`

**欢迎关注油管频道 “eSir playground” 观看相关的教学视频，并订阅我的油管频道[eSir Playground](https://www.youtube.com/c/esirplayground "esir playground")**:blush:

编译命令如下:

1. 首先装好 Ubuntu 64bit，推荐  Ubuntu  14 LTS x64  
2. 命令行输入

```bash
sudo apt-get update
```
然后输入以下的命令搭建系统环境

>sudo apt-get -y install build-essential asciidoc binutils bzip2 gawk gettext git libncurses5-dev patch unzip zlib1g-dev lib32gcc1 libc6-dev-i386 subversion flex node-uglify gcc-multilib p7zip p7zip-full msmtp libssl-dev texinfo libglib2.0-dev xmlto qemu-utils upx-ucl libelf-dev autoconf automake libtool autopoint

**如果你使用`root`执行了以上命令，那从此时开始，你必须使用`非root`权限用户进行后续操作**

3. 下载好源代码
```bash
git clone https://github.com/coolsnowwolf/lede
```
   然后进入`lede`目录（源码存在此目录，所以此目录即为`buildroot`目录）
```bash   
cd lede
```

4. 更新系统组件
```bash
./scripts/feeds update -a 
./scripts/feeds install -a
```
如果已经成功编译过，再次编译时还要同步L大的代码
```bash
git pull
```

5. 运行 `make menuconfig`入选单界面，选择CPU架构，型号，固件类型，所需插件及工具等，记得先`Save` 再退出
```bash
make menuconfig 
```

6. 下载源码文件到`buildroot`录下的`dl`目录
```bash
make download 
```

7. 正式开始编译，建议先运行`screen`命令守护进程，尤其是在`VPS`上编译时
```bash
make V=s
```
7-1. 如果你非常谨慎，也可以使用以下命令替换第7步的命令，来编译你的固件。`-j` 后面是线程数。第一次编译推荐用单线程，即 `-j1`，国内请尽量全局科学上网）
```bash
make -j1 V=s
```
-----   
本套代码保证肯定可以编译成功。里面包括了 R9 所有源代码，包括 IPK 的。

你可以自由使用，但源码编译二次发布请注明Lean大的 GitHub 仓库链接。谢谢合作！

特别提示：
1. 源代码中绝不含任何后门和可以监控或者劫持你的 HTTPS 的闭源软件，SSL 安全是互联网最后的壁垒。安全干净才是固件应该做到的。
2. 如果你自认为 Koolshare 论坛或者其固件的脑残粉，本人不欢迎你使用本源代码。所以如果你是，那么使用过程中遇到任何问题本人概不回应。
3. 如有问题需要讨论，欢迎加入 QQ 讨论群：Gargoyle OpenWrt 编译大群 ,号码 718010658 ，加群链接 点击链接加入群聊【Op共享技术交流群】：https://jq.qq.com/?_wv=1027&k=5PUGjOG

Before going to the next step，welcome subscribe youtube channel “佚sir playground” to obtain related video tutorials and more.

You need to have installed gcc, binutils, bzip2, flex, python, perl, make,
find, grep, diff, unzip, gawk, getopt, subversion, libz-dev and libc headers.

Run "./scripts/feeds update -a" to get all the latest package definitions
defined in feeds.conf / feeds.conf.default respectively
and "./scripts/feeds install -a" to install symlinks of all of them into
package/feeds/.

Please use "make menuconfig" to choose your preferred configuration for the toolchain and firmware.

And then run "make download" to download source code to buildroot dl dir.

At last Simply run the command "make V=s" to build your firmware.

It will build the cross-compile toolchain, the kernel and all choosen applications.

To build your own firmware you need to have access to a Linux, BSD or MacOSX system
(case-sensitive filesystem required). Cygwin will not be supported because of
the lack of case sensitiveness in the file system.



Note: Addition Lean's private package source code in ./package/lean directory. Use it under GPL v3.

GPLv3 is compatible with more licenses than GPLv2: 
it allows you to make combinations with code that has specific kinds of additional requirements that are not in GPLv3 itself. 
Section 7 has more information about this, including the list of additional requirements that are permitted.
