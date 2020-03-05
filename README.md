# OpenWrt 版本说明
+ 基于Lede OpenWrt 版本
+ 结合家庭x86软路由使用场景定制了软件包

对家庭路由高频功能进行了测试（x86软路由），保证可用。

## 家庭路由场景功能
+ 支持UPnP
+ 支持CIFS文件共享协议，挂载NAS、Samba、Windows文件夹
+ 支持单线多拨
+ 支持多拨负载均衡
+ 支持DNS去广告 + 去污染 + 最优访问速度筛选
+ 支持DDNS，可以通过域名随时获得家庭路由器IP
+ 支持SSH远程访问
+ 支持远程唤醒（WOL）
+ 支持定时唤醒（WOL）
+ 支持全功能Docker，可自由扩展功能（FQ）
+ 支持SSH远程访问
+ 支持SFTP，可通过常见SSH客户端随意传输文件
+ 安装OpenVMTools，优化在虚拟化环境运行速度
+ 支持百度网盘远程下载
+ 提供视觉效果较好的皮肤
+ 支持透明出国


感谢Lead，Lienol，CTCGFW等等作者。

===========================================================

中文：如何编译自己需要的 OpenWrt 固件

注意：
1. 不要用 root 用户 git 和编译！！！
2. 国内用户编译前最好准备好梯子
3. 默认登陆IP 192.168.1.1, 密码 password

编译命令如下:

1. 首先装好 Ubuntu 64bit，推荐  Ubuntu  14 LTS x64

2. 命令行输入 sudo apt-get update ，然后输入
sudo apt-get -y install build-essential asciidoc binutils bzip2 gawk gettext git libncurses5-dev libz-dev patch python3.5 unzip zlib1g-dev lib32gcc1 libc6-dev-i386 subversion flex uglifyjs git-core gcc-multilib p7zip p7zip-full msmtp libssl-dev texinfo libglib2.0-dev xmlto qemu-utils upx libelf-dev autoconf automake libtool autopoint device-tree-compiler g++-multilib linux-libc-dev:i386

3. git clone https://github.com/xiaoqingfengATGH/HomeLede.git homeLede命令下载好源代码，然后 cd homeLede 进入目录

4. ./scripts/feeds update -a 
   ./scripts/feeds install -a
   make menuconfig 

5. 最后选好你要的路由，输入 make -j1 V=s （-j1 后面是线程数。第一次编译推荐用单线程，国内请尽量全局科学上网）即可开始编译你要的固件了。

本套代码保证肯定可以编译成功。里面包括了 R9 所有源代码，包括 IPK 的。

你可以自由使用，但源码编译二次发布请注明我的 GitHub 仓库链接。谢谢合作！

去广告订阅地址默认内置来自以下源，如有去广告的误杀漏杀问题可以到这里报告：

https://github.com/privacy-protection-tools/anti-AD
