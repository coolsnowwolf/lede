# 欢迎来到 西瓜皮 LEDE 定制源码仓库

# 注：本项目fork自LEDE项目：https://github.com/coolsnowwolf/lede ，主要加入了对西瓜皮板子的定制化适配

## 西瓜皮官方讨论群
如有技术问题需要讨论或者交流，欢迎加入以下群：
1. QQ 讨论群： RK3568吃瓜 ,号码 136214897 ，加群链接：[点击加入](http://qm.qq.com/cgi-bin/qm/qr?_wv=1027&k=3GRrYTS2WPGKpwnkx_jhdhjwQ6SpkVMX&authKey=gDpBrz3hKSHLVSxHG%2BteK9zTqq5oyaPEFYngphDx0rxreDVlLs3BP8MLo4fPCVKV&noverify=0&group_code=136214897 "RK3568吃瓜")

<a href ="https://item.taobao.com/item.htm?spm=a1z10.1-c.w4004-1463280478.8.6145536bqhSAlY&id=711773675433
"><img src="https://github.com/6ang996/lede/blob/rk356x-xgp_support/doc/xgp.jpg?raw=true" width=600  /></a>

## 注意

1. **不要用 root 用户进行编译**
2. 国内用户编译前最好准备好梯子
3. 默认登陆IP 192.168.100.1（注：此处和LEDE项目默认IP有区别） 密码 password

## 编译命令

1. 首先装好 Linux 系统，推荐 Debian 11 或 Ubuntu LTS

2. 安装编译依赖

   ```bash
   sudo apt update -y
   sudo apt full-upgrade -y
   sudo apt install -y ack antlr3 aria2 asciidoc autoconf automake autopoint binutils bison build-essential \
   bzip2 ccache cmake cpio curl device-tree-compiler fastjar flex gawk gettext gcc-multilib g++-multilib \
   git gperf haveged help2man intltool libc6-dev-i386 libelf-dev libglib2.0-dev libgmp3-dev libltdl-dev \
   libmpc-dev libmpfr-dev libncurses5-dev libncursesw5-dev libreadline-dev libssl-dev libtool lrzsz \
   mkisofs msmtp nano ninja-build p7zip p7zip-full patch pkgconf python2.7 python3 python3-pip libpython3-dev qemu-utils \
   rsync scons squashfs-tools subversion swig texinfo uglifyjs upx-ucl unzip vim wget xmlto xxd zlib1g-dev
   ```

3. 下载源代码，更新 feeds 并选择配置

   ```bash
   git clone https://github.com/6ang996/lede
   cd lede
   ./scripts/feeds update -a
   ./scripts/feeds install -a
   #此处增加了西瓜皮板子的默认.config配置
   cp custom_config/rk3568_xgp_config .config
   make defconfig
   make menuconfig
   ```

4. 下载 dl 库，编译固件
（-j 后面是线程数，第一次编译推荐用单线程）

   ```bash
   make V=s download -j16
   make V=s -j$(nproc) || make V=sc -j1
   ```

二次编译：

```bash
cd lede
make V=s -j$(nproc) || make V=sc -j1
```

如果需要重新配置：

```bash
rm -rf ./tmp && rm -rf .config
#此处增加了西瓜皮板子的默认.config配置
cp custom_config/rk3568_xgp_config .config
make defconfig
make menuconfig
make V=s -j$(nproc) || make V=sc -j1
```

编译完成后西瓜皮固件输出路径：bin/targets/rockchip/armv8/openwrt-rockchip-armv8-nlnet_xgp-squashfs-sysupgrade.img.gz


## 特别提示（注：以下为LEDE仓库提示内容）

1. 源代码中绝不含任何后门和可以监控或者劫持你的 HTTPS 的闭源软件， SSL 安全是互联网最后的壁垒。安全干净才是固件应该做到的；

2. 想学习 OpenWrt 开发，但是摸不着门道？自学没毅力？基础太差？怕太难学不会？跟着佐大学 OpenWrt 开发入门培训班助你能学有所成
报名地址：[点击报名](http://forgotfun.org/2018/04/openwrt-training-2018.html "报名")

3. QCA IPQ60xx 开源仓库地址：<https://github.com/coolsnowwolf/openwrt-gl-ax1800>

4. 存档版本仓库地址：<https://github.com/coolsnowwolf/openwrt>


## LEDE项目捐贈（注：以下为LEDE仓库捐赠内容）

如果你觉得LEDE项目对你有帮助，可以捐助LEDE开发者，以鼓励LEDE项目能持续发展，更加完善

 ![star](doc/star.png) 
