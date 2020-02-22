# Welcome to source code of Ryuchen's OpenWrt 

本套代码fork自[coolsnowwolf/lede](https://github.com/coolsnowwolf/lede)，仅限自己维护和使用

## [![Build](https://img.shields.io/github/workflow/status/coolsnowwolf/lede/OpenWrt-CI/master?color=blue)](https://github.com/coolsnowwolf/lede/actions?query=workflow%3AOpenWrt-CI) [![Contributors](https://img.shields.io/github/contributors/coolsnowwolf/lede?color=blue)](https://github.com/coolsnowwolf/lede/graphs/contributors) [![Last commit](https://img.shields.io/github/last-commit/coolsnowwolf/lede?color=blue)](https://github.com/coolsnowwolf/lede/commits/master)

以下引用冷雪狼的 README.md 的说明

本套代码保证肯定可以编译成功。里面包括了 R9 所有源代码，包括 IPK 的。
> This set of code is guaranteed to compile successfully. It includes all the source code of R9, including the IPK.

你可以自由使用，但源码编译二次发布请注明我的 GitHub 仓库链接。谢谢合作！
> You can use it freely, but please indicate the link to my GitHub repository for the second release of source code compilation. Thank you for your cooperation!

> 注意：\
    * 不要用 root 用户进行 git clone 和 编译 ！！！\
    * 国内用户编译前最好准备好梯子 \
    * 默认登陆IP: 192.168.86.1, 密码: password

> Notice: \
    * Don't git clone and compile as root ！！！\
    * Default Login IP: 192.168.86.1, Default Login Password: password

## 编译方式(Method to Compile)

### 首先装好 Ubuntu 64bit，推荐  Ubuntu 18.04 LTS X86_64

    > linux 平台下编译固件最推荐 ~
    > windows 平台建议使用 WSL2(require version 2004) * 虚拟机性能几乎没有损失
    > macOS 平台安装 GNU 软件包之后进行编译(官方这里不作推荐)

### 打开 Ubuntu Terminal 然后输入

* 进行源码的 clone

```bash
git clone https://github.com/coolsnowwolf/lede.git where/you/want/to/place
```

* 环境的更新和依赖包的安装
  
```bash
sudo apt update && apt upgrade && apt autoremove

sudo apt-get -y install build-essential asciidoc binutils bzip2 gawk gettext git libncurses5-dev libz-dev patch python3 unzip zlib1g-dev lib32gcc1 libc6-dev-i386 subversion flex uglifyjs git-core gcc-multilib p7zip p7zip-full msmtp libssl-dev texinfo libglib2.0-dev xmlto qemu-utils upx libelf-dev autoconf automake libtool autopoint device-tree-compiler
```

* 更新 feeds

```bash
cd where/you/want/to/place
./scripts/feeds update -a
./scripts/feeds install -a
```

> 说明 \
> `./scripts/feeds update -a` 将获取定义在feeds.conf和feeds.conf.default中所有的包信息 \
> `./scripts/feeds update -a` will to get all the latest package definitions
defined in feeds.conf / feeds.conf.default respectively. \
> `./scripts/feeds install -a` 将安装上述获取的所有包的软连接 \
> `./scripts/feeds install -a` will to install symlinks of all of them into
package/feeds/. \

* 进行可视化配置
  
```bash
make menuconfig
```

* 进行固件编译

```bash
make -j1 V=s （-j1 后面是线程数。第一次编译推荐用单线程，国内请尽量全局科学上网）
```

## 特别提示

* 源代码中绝不含任何后门和可以监控或者劫持你的 HTTPS 的闭源软件，SSL 安全是互联网最后的壁垒。安全干净才是固件应该做到的；
* 如有技术问题需要讨论，欢迎加入 QQ 讨论群：OP共享技术交流群 ,号码 297253733 ，加群链接: 点击链接加入群聊【OP共享技术交流群】：jq.qq.com/?_wv=1027&k=5yCRuXL
* 想学习OpenWrt开发，但是摸不着门道？自学没毅力？基础太差？怕太难学不会？跟着佐大学OpenWrt开发入门培训班助你能学有所成
报名地址：http://forgotfun.org/2018/04/openwrt-training-2018.html

## 操作警告

* 针对Github小白用户，特别提醒，当你在更新你自己的fork项目时，注意使用pull request的方向性，不要进行因为更新源码导致的无效pull request！！！
* 每个人遇到问题的时候都会希望通过blog找到完整的步骤解答，但是既然你都参与和使用这个项目了，想必你一定是个爱折腾的人，否者你也不会想到使用源码编译，做个伸手党就好了，既然如此请遇到问题优先自己google解决，google远比作者更强大~
* 最后提交问题之前请一定先去看看这个地方[How-To-Ask-Questions-The-Smart-Way](https://github.com/ryanhanwu/How-To-Ask-Questions-The-Smart-Way)，既然你都无法解决这个问题了，在烦请作者解决这个问题的时候更好的为你服务，也同时让别人更好的知道你的问题，如果看上去阐述不明确的issue，将一律关闭，不进行处理~

## 开源声明(License)

Copyright © 2006~2020 [coolsnowwolf](https://github.com/coolsnowwolf). \
本项目是在 [GPLv3](https://github.com/coolsnowwolf/lede/raw/master/LICENSE) 开源协议下进行开发和制作的，欢迎各位参与其中，但是请谨遵开源协议，谢谢合作！

Copyright © 2006~2020 [coolsnowwolf](https://github.com/coolsnowwolf). \
This project is under [GPLv3](https://github.com/coolsnowwolf/lede/raw/master/LICENSE) License. Everyone is welcome to participate, but please follow the open source agreement, thank you for your cooperation!
