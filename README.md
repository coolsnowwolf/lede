IP 10.10.10.10  密码 password

首次编译：
-
1.  Ubuntu  18 LTS x64

2. 环境准备

   `sudo apt-get update`

   `sudo apt-get -y install build-essential asciidoc binutils bzip2 gawk gettext git libncurses5-dev libz-dev patch python3 python2.7 unzip zlib1g-dev lib32gcc1 libc6-dev-i386 subversion flex uglifyjs git-core gcc-multilib p7zip p7zip-full msmtp libssl-dev texinfo libglib2.0-dev xmlto qemu-utils upx libelf-dev autoconf automake libtool autopoint device-tree-compiler g++-multilib antlr3 gperf wget curl swig rsync`

3. `git clone https://github.com/eramicro/openwrt openwrt` 下载源代码，然后进入目录中。

4. ```bash

   下载feeds源中的软件包源码
   
    ./scripts/feeds update -a
   
   安装feeds中的软件包
   
    ./scripts/feeds install -a
   
   调整 OpenWrt 系统组件
   
     make menuconfig

5. 预下载编译所需的软件包

    `make -j8 download V=s`
   
6. 检查文件完整性

    `find dl -size -1024c -exec ls -l {} \;`
   
   此命令列出下载不完整的文件，如果存在这样的文件可以使用find dl -size -1024c -exec rm -f {} \;命令将它们删除，然后重新执行make download下载并反复检查，确认所有文件完整可大大提高编译成功率。
   
7. 开始编译

    `make -j1 V=s` 
 
    -j1 后面是线程数。第一次编译推荐用单线程。




二次编译：
-

```bash
cd lede
git pull
./scripts/feeds update -a && ./scripts/feeds install -a
./scripts/diffconfig.sh>diffconfig
make defconfig
make -j8 download
find dl -size -1024c -exec ls -l {} \;
make -j$(nproc) || make -j1 || make -j1 V=s
```

如果需要重新配置：
```bash
rm -rf ./tmp && rm -rf .config
make menuconfig
find dl -size -1024c -exec ls -l {} \;
make -j$(($(nproc) + 1)) V=s
```

文件清理
-
```
make clean

清除旧的编译产物（可选）（在源码有大规模更新或者内核更新后执行，以保证编译质量。此操作会删除/bin和/build_dir目录中的文件）。


make dirclean

清除旧的编译产物、交叉编译工具及工具链等目录（可选）（更换架构编译前必须执行。此操作会删除/bin和/build_dir目录的中的文件(make clean)以及/staging_dir、/toolchain、/tmp和/logs中的文件）。


make distclean

清除 OpenWrt 源码以外的文件（可选）（除非是做开发，并打算 push 到 GitHub 这样的远程仓库，否则几乎用不到。此操作相当于make dirclean外加删除/dl、/feeds目录和.config文件）。


git clean -xdf

还原 OpenWrt 源码到初始状态（可选）（如果把源码改坏了，或者长时间没有进行编译时使用）。


rm -rf tmp

清除临时文件（删除执行make menuconfig后产生的一些临时文件，包括一些软件包的检索信息，删除后会重新加载package目录下的软件包。若不删除会导致一些新加入的软件包不显示）。


rm -f .config

删除编译配置文件（在不删除的情况下如果取消选择某些组件它的依赖组件不会自动取消，所以对于需要调整组件的情况下建议删除）。
```
编译完成后输出路径：/lede/bin/targets
