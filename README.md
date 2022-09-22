# 《项目指北？》

## 项目简介
用途：这里主要是用于云编译+UA2F防检测部署
其他用处：可以自定义编译自己想要的openwrt固件

## 项目鸣谢
- [Zxilly](https://github.com/Zxilly)大佬的项目[UA2F](https://github.com/Zxilly/UA2F)
这是Z佬的官方部署教程:[OpenWrt 编译与防检测部署教程](https://sunbk201public.notion.site/sunbk201public/OpenWrt-f59ae1a76741486092c27bc24dbadc59)
- [MoorCorPa](https://github.com/MoorCorPa)大佬的项目例程:[Actions-immortalWrt-UA2F](https://github.com/MoorCorPa/Actions-immortalWrt-UA2F)
- [P3TERX](https://github.com/P3TERX)的编译模板:[Actions-OpenWrt](https://github.com/P3TERX/Actions-OpenWrt)
这是P佬的官方部署博客:[使用 GitHub Actions 云编译 OpenWrt](https://p3terx.com/archives/build-openwrt-with-github-actions.html)

## 分支介绍
- [master分支](https://github.com/SoDebug/OpenWRTResourseCode)用于同步[lede](https://github.com/coolsnowwolf/lede)的仓库源码，保持编译源始终保持最新~~还没写这部分的代码~~
- [dev分支](https://github.com/SoDebug/OpenWRTResourseCode/tree/dev)包含了自定义编译的脚本，在后面会详细介绍，其默认使用``dev分支``的代码进行编译(这是为了防止新源码有故障，在不能保证源码可用情况下不要立即同步最新源)
- [k2p分支](https://github.com/SoDebug/OpenWRTResourseCode/tree/k2p)专用于编译k2p固件，其实和``dev分支``区别不大，就是更改了编译固件型号，几行代码的区别

## 使用办法
### 这里以``dev分支``为例
- ``device_config``:用于存放编译固件型号的文件夹
- ``device_info.config``:用于基本的设备配置文件（包含设备的型号、需要启用的插件）
代码内容如下：
```
# 以下三行代码用于配置固件型号
CONFIG_TARGET_x86=y
CONFIG_TARGET_x86_64=y
CONFIG_TARGET_x86_64_DEVICE_generic=y

# 以下代码用于配置固件所需要插件
CONFIG_PACKAGE_ua2f=y
CONFIG_PACKAGE_ipset=y
CONFIG_PACKAGE_iptables-mod-conntrack-extra=y
CONFIG_PACKAGE_iptables-mod-filter=y
CONFIG_PACKAGE_iptables-mod-ipopt=y
CONFIG_PACKAGE_iptables-mod-nfqueue=y			   
CONFIG_PACKAGE_iptables-mod-u32=y

CONFIG_PACKAGE_kmod-ipt-conntrack-extra=y
CONFIG_PACKAGE_kmod-ipt-filter=y
CONFIG_PACKAGE_kmod-ipt-ipopt=y
CONFIG_PACKAGE_kmod-ipt-nfqueue=y			   
CONFIG_PACKAGE_kmod-ipt-u32=y
CONFIG_PACKAGE_kmod-nfnetlink-queue=y

CONFIG_LUCI_LANG_zh_Hans=y
CONFIG_PACKAGE_luci-theme-argon=y
CONFIG_PACKAGE_luci-app-argon-config=y
```
- ``outer_repo.sh``在更新与安装 feeds 的前执行,作用是拉取仓库里没有的插件的源码
内容如下：
```
#!/bin/bash
#
# Copyright (c) 2019-2020 P3TERX <https://p3terx.com>
#
# This is free software, licensed under the MIT License.
# See /LICENSE for more information.
#
# https://github.com/P3TERX/Actions-OpenWrt
# File name: diy-part1.sh
# Description: OpenWrt DIY script part 1 (Before Update feeds)
#

# Uncomment a feed source
#sed -i 's/^#\(.*helloworld\)/\1/' feeds.conf.default

# Add a feed source
#sed -i '$a src-git lienol https://github.com/Lienol/openwrt-package' feeds.conf.default
git clone https://github.com/CHN-beta/rkp-ipid package/rkp-ipid
git clone https://github.com/Zxilly/UA2F package/UA2F
```
- ``kernel_mod.sh``在更新与安装 feeds 的后执行，作用是配置一些内核层面的修改
代码如下：
```
#!/bin/bash
#
# Copyright (c) 2019-2020 P3TERX <https://p3terx.com>
#
# This is free software, licensed under the MIT License.
# See /LICENSE for more information.
#
# https://github.com/P3TERX/Actions-OpenWrt
# File name: diy-part2.sh
# Description: OpenWrt DIY script part 2 (After Update feeds)
#

# CONFIG_NETFILTER=y
# CONFIG_NETFILTER_NETLINK=y
# CONFIG_NETFILTER_NETLINK_GLUE_CT=y
# CONFIG_NETFILTER_NETLINK_LOG=y
# CONFIG_NF_CONNTRACK=y
# CONFIG_NF_CT_NETLINK=y

# Modify default IP
#sed -i 's/192.168.1.1/192.168.50.5/g' package/base-files/files/bin/config_generate
target=$(grep "^CONFIG_TARGET" .config --max-count=1 | awk -F "=" '{print $1}' | awk -F "_" '{print $3}')
for configFile in $(ls target/linux/$target/config*)
do
    echo -e "\nCONFIG_NETFILTER_NETLINK_GLUE_CT=y" >> $configFile
done
```
<center>以上就是基本的使用，明白以上可以保证构建出可以使用的固件</center>

## 进阶？
### ``yml``文件变量的作用以及配置（来自[使用 GitHub Actions 云编译 OpenWrt](https://p3terx.com/archives/build-openwrt-with-github-actions.html)）
|环境变量|功能|默认值|
|:----:|:----:|:----:|
|REPO_URL|源码仓库地址|https://github.com/SoDebug/OpenWRTResourseCode|
|REPO_BRANCH|源码分支|dev|
|FEEDS_CONF|自定义feeds.conf.default文件名|feeds.conf.default|
|CONFIG_FILE|自定义.config文件名|device_config/device_info.config|
|DIY_P1_SH|自定义diy-part1.sh文件名|device_config/outer_repo.sh|
|DIY_P2_SH|自定义diy-part2.sh文件名|device_config/kernel_mod.sh|
|UPLOAD_BIN_DIR|上传 bin 目录。即包含所有 ipk 文件和固件的目录。|false|
|UPLOAD_FIRMWARE|上传固件目录。|false|
|UPLOAD_COWTRANSFER|上传固件到奶牛快传。|false|
|UPLOAD_WERANSFER|上传固件到 WeTransfer 。|false|
|UPLOAD_RELEASE|上传固件到 releases 。|false|
|TZ|时区设置|Asia/Shanghai|

### 源码更新自动编译
- 在``Github``账户设置中找到``Developer Settings``，创建 ``Personal access token(PAT)`` ，勾选``repo``权限，这将用于自动触发编译工作流程。
- 然后点击自己仓库的``Settings``选项卡，再点击``Secrets``。添加名为``ACTIONS_TRIGGER_PAT``的加密环境变量，保存刚刚创建的``PAT``。
- 在 ``Actions`` 页面选择``Update Checker``，点击``Run workflow``手动进行一次测试运行。如果没有报错且 ``OpenWrt`` 编译工作流程被触发，则代表测试通过。
- 最后编辑``Update Checker``的 ``workflow`` 文件（``.github/workflows/update-checker.yml``），取消注释（删除#）定时触发相关的部分。这里可以根据 ``cron`` 格式来设定检测的时间，时区为 UTC 。
```
#  schedule:
#    - cron: 0 */18 * * *
```

## 以下是Lede的原版介绍

如何编译自己需要的 OpenWrt 固件 [How to build your Openwrt firmware](./README_EN.md)

## 官方讨论群
如有技术问题需要讨论或者交流，欢迎加入以下群：
1. QQ 讨论群： Op固件技术研究群 ,号码 891659613 ，加群链接：[点击加入](https://jq.qq.com/?_wv=1027&k=XL8SK5aC "Op固件技术研究群")
2. TG 讨论群： OP 编译官方大群 ，加群链接：[点击加入](https://t.me/JhKgAA6Hx1 "OP 编译官方大群")

## 注意

1. **不要用 root 用户进行编译**
2. 国内用户编译前最好准备好梯子
3. 默认登陆IP 192.168.1.1 密码 password

## 编译命令

1. 首先装好 Linux 系统，推荐 Debian 11 或 Ubuntu LTS

2. 安装编译依赖

   ```bash
   sudo apt update -y
   sudo apt full-upgrade -y
   sudo apt install -y ack antlr3 asciidoc autoconf automake autopoint binutils bison build-essential \
   bzip2 ccache cmake cpio curl device-tree-compiler fastjar flex gawk gettext gcc-multilib g++-multilib \
   git gperf haveged help2man intltool libc6-dev-i386 libelf-dev libglib2.0-dev libgmp3-dev libltdl-dev \
   libmpc-dev libmpfr-dev libncurses5-dev libncursesw5-dev libreadline-dev libssl-dev libtool lrzsz \
   mkisofs msmtp nano ninja-build p7zip p7zip-full patch pkgconf python2.7 python3 python3-pip libpython3-dev qemu-utils \
   rsync scons squashfs-tools subversion swig texinfo uglifyjs upx-ucl unzip vim wget xmlto xxd zlib1g-dev
   ```

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

本套代码保证肯定可以编译成功。里面包括了 R22 所有源代码，包括 IPK 的。

你可以自由使用，但源码编译二次发布请注明我的 GitHub 仓库链接。谢谢合作！

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

### 如果你使用 WSL/WSL2 进行编译

由于 WSL 的 PATH 中包含带有空格的 Windows 路径，有可能会导致编译失败，请在 `make` 前面加上：

```bash
PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
```

### macOS 原生系统进行编译

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

## 特别提示

1. 源代码中绝不含任何后门和可以监控或者劫持你的 HTTPS 的闭源软件， SSL 安全是互联网最后的壁垒。安全干净才是固件应该做到的；

2. 想学习 OpenWrt 开发，但是摸不着门道？自学没毅力？基础太差？怕太难学不会？跟着佐大学 OpenWrt 开发入门培训班助你能学有所成
报名地址：[点击报名](http://forgotfun.org/2018/04/openwrt-training-2018.html "报名")

3. QCA IPQ60xx 开源仓库地址：<https://github.com/coolsnowwolf/openwrt-gl-ax1800>

4. 存档版本仓库地址：<https://github.com/coolsnowwolf/openwrt>

## 软路由介绍

iKOOLCORE 硬酷R1 多网口小主机 - N5105/N6005 : Cube box, rest fun. 方寸之间，尽是乐趣

(商品介绍页面 - 深圳市硬酷科技)：
[Lean用户R1专属券](https://taoquan.taobao.com/coupon/unify_apply.htm?sellerId=1981117776&activityId=3eb8a2ad3c8945ed831b203f44dc218b)
[N5105下单链接](https://item.taobao.com/item.htm?ft=t&id=682987219699)
[N6005下单链接](https://item.taobao.com/item.htm?ft=t&id=682672722483)  

(商品介绍页面 - 硬酷科技（支持花呗）)：
[Lean用户R1专属券](https://taoquan.taobao.com/coupon/unify_apply.htm?sellerId=2208215115814&activityId=e013790422b849edb3a71e4e6aa46ff3)
[下单链接](https://item.taobao.com/item.htm?ft=t&id=682987219699)

[![r1](doc/r1.jpg)](https://item.taobao.com/item.htm?ft=t&id=682025492099)

## 捐贈

如果你觉得此项目对你有帮助，可以捐助我们，以鼓励项目能持续发展，更加完善

 ![star](doc/star.png) 
