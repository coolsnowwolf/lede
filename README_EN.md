Welcome to Lean's git source of OpenWrt and packages
=

How to build your Openwrt firmware.
-
Note:
--
1. DO **NOT** USE **root** USER FOR COMPILING!!!

2. Users within China should prepare proxy before building.

3. Web admin panel default IP is 192.168.1.1 and default password is "password".

Let's start!
---
1. First, install Ubuntu 64bit (Ubuntu 18 LTS x86 is recommended).

2. Run `sudo apt-get update` in the terminal, and then run
    `
    sudo apt-get -y install build-essential asciidoc binutils bzip2 gawk gettext git libncurses5-dev libz-dev patch python3 python2.7 unzip zlib1g-dev lib32gcc1 libc6-dev-i386 subversion flex uglifyjs git-core gcc-multilib p7zip p7zip-full msmtp libssl-dev texinfo libglib2.0-dev xmlto qemu-utils upx libelf-dev autoconf automake libtool autopoint device-tree-compiler g++-multilib antlr3 gperf wget curl swig rsync
    `

3. Run `git clone https://github.com/coolsnowwolf/lede` to clone the source code, and then `cd lede` to enter the directory

4. ```bash
   ./scripts/feeds update -a
   ./scripts/feeds install -a
   make menuconfig
   ```

5. Run `make -j8 download V=s` to download libraries and dependencies (user in China should use global proxy when possible)

6. Run `make -j1 V=s` (integer following -j is the thread count, single-thread is recommended for the first build) to start building your firmware.

This source code is promised to be compiled successfully.

You can use this source code freely, but please link this GitHub repository when redistributing. Thank you for your cooperation!
=

Rebuild:
```bash
cd lede
git pull
./scripts/feeds update -a && ./scripts/feeds install -a
make defconfig
make -j8 download
make -j$(($(nproc) + 1)) V=s
```

If reconfiguration is need:
```bash
rm -rf ./tmp && rm -rf .config
make menuconfig
make -j$(($(nproc) + 1)) V=s
```

Build result will be produced to `bin/targets` directory.

Special tips:
------
1. This source code doesn't contain any backdoors or close source applications that can monitor/capture your HTTPS traffic, SSL is the final castle of cyber security. Safety is what a firmware should achieve.

2. If you have any technical problem, you may join the QQ discussion group: 297253733, link: click [here](https://jq.qq.com/?_wv=1027&k=5yCRuXL)

3. Want to learn OpenWrt development but don't know how? Can't motivate yourself for self-learning? Not enough fundamental knowledge? Learn OpenWrt development with Mr. Zuo through his Beginner OpenWrt Training Course. Click [here](http://forgotfun.org/2018/04/openwrt-training-2018.html) to register.

## Router Recommendation
Not Sponsored: If you are finding a low power consumption, small and performance promising x86/x64 router, I personally recommend the 
EZPROv1 Alumium Edition (N3710 4000M): [Details](https://item.taobao.com/item.htm?spm=a230r.1.14.20.144c763fRkK0VZ&id=561126544764)

![xm1](doc/xm5.jpg)
![xm2](doc/xm6.jpg)

## Donation

If this project does help you, please consider donating to support the development of this project.

### Alipay

![alipay](doc/alipay_donate.jpg)

### WeChat

![wechat](doc/wechat_donate.jpg)

## Note: Addition Lean's private package source code in `./package/lean` directory. Use it under GPL v3.

## GPLv3 is compatible with more licenses than GPLv2: it allows you to make combinations with code that has specific kinds of additional requirements that are not in GPLv3 itself. Section 7 has more information about this, including the list of additional requirements that are permitted.
