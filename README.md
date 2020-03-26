源码Fork来自于：
https://github.com/coolsnowwolf/lede.git

中文：如何编译自己需要的 OpenWrt 固件
-
注意：
-
1. 不要用 **root** 进行 git 和编译！！！
2. 国内用户编译前最好准备好梯子
3. 默认登陆IP 192.168.10.1, 密码 password

编译命令如下:
-
1. 首先装好 Ubuntu 64bit，推荐  Ubuntu  18 LTS x64 

2. 命令行输入 `sudo apt-get update` ，然后输入
`
sudo apt-get -y install build-essential asciidoc binutils bzip2 gawk gettext git libncurses5-dev libz-dev patch python3.5 unzip zlib1g-dev lib32gcc1 libc6-dev-i386 subversion flex uglifyjs git-core gcc-multilib p7zip p7zip-full msmtp libssl-dev texinfo libglib2.0-dev xmlto qemu-utils upx libelf-dev autoconf automake libtool autopoint device-tree-compiler g++-multilib 
`

3. 使用 `git clone https://github.com/leaus/lede` 命令下载好源代码，然后 `cd lede` 进入目录 

4. ```bash
   ./scripts/feeds update -a 
   ./scripts/feeds install -a
   make menuconfig 
   ```

5. `make download v=s` 下载dl库（国内请尽量全局科学上网）


6. 输入 `make -j1 V=s` （-j1 后面是线程数。第一次编译推荐用单线程）即可开始编译你要的固件了。 

本套代码保证肯定可以编译成功。里面包括了 R20 所有源代码，包括 IPK 的。 

二次编译：

`cd lede`                                                    
`git pull`                                                         
`./scripts/feeds update -a && ./scripts/feeds install -a` 
 `make defconfig`                                                  
`make -jn V=s`  n=线程数+1，例如4线程的I5填-j5，开始编译                                                  

需要重新配置：

`rm -rf ./tmp && rm -rf .config`                                   
`make menuconfig`                                                  
`make -jn V=s`    n=线程数+1，例如4线程的I5填-j5，开始编译                                                   

编译完成后输出路径：/lede/bin/targets
