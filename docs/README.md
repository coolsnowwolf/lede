Welcome to Lean's  git source of OpenWrt and packages

中文：如何编译自己需要的 OpenWrt 固件

# 目录 <!-- omit in toc --> 
- [注意](#%e6%b3%a8%e6%84%8f)
- [首次编译](#%e9%a6%96%e6%ac%a1%e7%bc%96%e8%af%91)
  - [二次编译](#%e4%ba%8c%e6%ac%a1%e7%bc%96%e8%af%91)
- [一些说明](#%e4%b8%80%e4%ba%9b%e8%af%b4%e6%98%8e)


# 注意
1. 不要用 root 用户 git 和编译！！！
2. 国内用户编译前最好准备好梯子
3. 默认登陆IP 192.168.1.1, 密码 password

# 首次编译

编译命令如下:

1. 首先装好 Ubuntu 64bit，推荐  Ubuntu  14 LTS x64

2. 执行如下命令安装必备包 
    ```shell
    sudo apt-get update
    ```

    之后输入

    ```shell
    sudo apt-get -y install build-essential asciidoc binutils bzip2 gawk gettext git libncurses5-dev libz-dev patch unzip zlib1g-dev lib32gcc1 libc6-dev-i386 subversion flex uglifyjs git-core gcc-multilib p7zip p7zip-full msmtp libssl-dev texinfo libglib2.0-dev xmlto qemu-utils upx libelf-dev autoconf automake libtool autopoint
    ```

3. 设置Ubuntu代理


   **例如你局域网中具有10.10.0.3的ss或v2ray，并且开启了允许局域网链接，http代理端口1081**

   <br>

   - 针对所有用户设置默认代理
        ```shell
        sudo nano /etc/environment
        ```
        添加如下内容
        ```shell
        http_proxy="http://10.10.0.3:1081/"
        https_proxy="http://10.10.0.3:1081/"
        ftp_proxy="http://10.10.0.3:1081/"
        no_proxy="localhost,127.0.0.1,::1"
        ```
    - 设置APT程序包管理器的代理
        ```shell
        sudo nano /etc/apt/apt.conf.d/80proxy
        ```
        添加如下信息
        ```shell
        Acquire::http::proxy "http://10.10.0.3:1081/";
        Acquire::https::proxy "https://10.10.0.3:1081/";
        Acquire::ftp::proxy "ftp://10.10.0.3:1081/";
        ```
    - 设置仅适用于wget的代理
        ```shell
        nano ~/.wgetrc
        ```
        添加如下设置
        ```shell
        use_proxy = on
        http_proxy = http://10.10.0.3:1081/ 
        https_proxy = http://10.10.0.3:1081/ 
        ftp_proxy = http://10.10.0.3:1081/ 
        ```
    - 设置git代理
        ```shell
        git config --global http.proxy http://10.10.0.3:1081
        ```
    - 针对CLI系统级代理
        ```shell
        sudo nano  /etc/profile.d/proxy.sh
        ```
        添加如下内容
        ```shell
        # set proxy config via profie.d - should apply for all users
        # 
        export http_proxy="http://10.10.0.3:1081/"
        export https_proxy="http://10.10.0.3:1081/"
        export ftp_proxy="http://10.10.0.3:1081/"
        export no_proxy="127.0.0.1,localhost"

        # For curl
        export HTTP_PROXY="http://10.10.0.3:1081/"
        export HTTPS_PROXY="http://10.10.0.3:1081/"
        export FTP_PROXY="http://10.10.0.3:1081/"
        export NO_PROXY="127.0.0.1,localhost"
        ```
        执行命令
        ```shell
        sudo chmod +x  /etc/profile.d/proxy.sh
        ```
        ```shell
        source /etc/profile.d/proxy.sh
        ```
        确认信息
        ```shell
        env | grep -i proxy
        ```

   
   
4. 通过git指令克隆源码
   ```shell
   git clone https://github.com/coolsnowwolf/lede
   ```
   执行命令 `cd lede` 进入目录

5. 执行指令更新feeds中定义的包，并安装
   ```shell
   ./scripts/feeds update -a 
   ./scripts/feeds install -a
   make menuconfig
   ``` 
6. 最后选好你要的路由，执行
   ```shell
   make -j1 V=s
   ```
   进行编译，首次编译可能会持续4-6个小时，时间长短取决于编译机器配置与网络质量。

    （-j1 后面是线程数。第一次编译推荐用单线程，国内请尽量全局科学上网）

## 二次编译
1. 进入项目lede目录，`cd lede`
2. 二次编译请首先执行如下命令，**清理上次编译的object文件(后缀.o的文件))及可执行文件**
   ```shell
   make clean
   ```
   或执行如下命令，**清理上次编译的生成所有文件**
   ```shell
   make distclean
   ```
3. 使用git指令，更新源码到最新版本
   ```shell
   git pull
   ```
4. 执行首次编译中的5至6步即可

# 一些说明


本套代码保证肯定可以编译成功。里面包括了 R9 所有源代码，包括 IPK 的。

你可以自由使用，但源码编译二次发布请注明我的 GitHub 仓库链接。谢谢合作！

特别提示：
1. 源代码中绝不含任何后门和可以监控或者劫持你的 HTTPS 的闭源软件，SSL 安全是互联网最后的壁垒。安全干净才是固件应该做到的。
2.如果你自认为 Koolshare 论坛或者其固件的脑残粉，本人不欢迎你使用本源代码。所以如果你是，那么使用过程中遇到任何问题本人概不回应。
3.如有问题需要讨论，欢迎加入 QQ 讨论群：Gargoyle OpenWrt 编译大群 ,号码 718010658 ，加群链接 点击链接加入群聊【Op共享技术交流群】：https://jq.qq.com/?_wv=1027&k=5PUGjOG

Please use "make menuconfig" to choose your preferred
configuration for the toolchain and firmware.

You need to have installed gcc, binutils, bzip2, flex, python, perl, make,
find, grep, diff, unzip, gawk, getopt, subversion, libz-dev and libc headers.

Run "./scripts/feeds update -a" to get all the latest package definitions
defined in feeds.conf / feeds.conf.default respectively
and "./scripts/feeds install -a" to install symlinks of all of them into
package/feeds/.

Use "make menuconfig" to configure your image.

Simply running "make" will build your firmware.
It will download all sources, build the cross-compile toolchain, 
the kernel and all choosen applications.

To build your own firmware you need to have access to a Linux, BSD or MacOSX system
(case-sensitive filesystem required). Cygwin will not be supported because of
the lack of case sensitiveness in the file system.



Note: Addition Lean's private package source code in ./package/lean directory. Use it under GPL v3.

GPLv3 is compatible with more licenses than GPLv2: it allows you to make combinations with code that has specific kinds of additional requirements that are not in GPLv3 itself. Section 7 has more information about this, including the list of additional requirements that are permitted.

