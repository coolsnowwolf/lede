# 欢迎来到 Lean 的 LEDE 源码仓库

为国产龙芯 LOONGSON SoC loongarch64 / 飞腾 Phytium 腾锐 D2000 系列架构添加支持

I18N: [English](README_EN.md) | [简体中文](README.md) | [日本語](README_JA.md)

## 官方讨论群

如有技术问题需要讨论或者交流，欢迎加入以下群：

1. QQ 讨论群：Op 固件技术研究群，号码 891659613，加群链接：[点击加入](https://qm.qq.com/q/IMa6Yf2SgC "Op固件技术研究群")
2. TG 讨论群：OP 编译官方大群，加群链接：[点击加入](https://t.me/JhKgAA6Hx1 "OP 编译官方大群")

## 瑞莎网络计算 - 轻松打造你的网络边缘计算系统

![radxa-netcompute-features](./doc/radxa.png)

瑞莎网络计算系列覆盖从入门到高性能的边缘网关场景，从千兆到 2.5G，从四核到六核，全系列基于瑞芯微高可靠处理器，低功耗、稳定运行，适合 7×24 小时网络服务。

| 产品型号  | E20C                    | E24C                    | E25                             | E52C                              | E54C                      |
| -------- | ----------------------- | ----------------------- | ------------------------------- | --------------------------------- | ------------------------- |
| 处理器    | 瑞芯微RK3528A（四核）     | 瑞芯微 RK3528A（四核）     | 瑞芯微 RK3568（四核）             | 瑞芯微 RK3582（六核）               | 瑞芯微 RK3582（六核）        |
| 以太网    | 2x 千兆以太网            | 4x 千兆以太网              | 2x 2.5G 以太网                  | 2x 2.5G 以太网                     | 4x 千兆以太网               |
| 适用场景  | 入门级软路由 / 家用网关    | 多口家庭路由 / IoT 网关     | 高速 2.5G 家庭 / 小型企业软路由    | 高性能 2.5G 路由 / 边缘计算 / VPN    | 多口网络控制器 / 企业网关     |

- 多网口设计，为网络计算而生
  - 提供 2 / 4 网口、千兆 / 2.5G 多种组合，灵活适配家用、宿舍、小型企业与 IoT 网络。
- 一板多用，系统自由切换
  - 支持 OpenWrt、Debian、Armbian 等多种系统，可应用于网关、轻量服务器或开发板等领域。
- 稳定可靠，适合全天候运行
  - 优秀的金属外壳散热设计 + 瑞芯微平台的低功耗特性，确保长时间满载仍能保持稳定。
- 开发生态完善，轻松扩展功能
  - 提供配套文档、源码与社区支持，方便开发者进行二次开发、插件扩展与自定义功能。

## 了解更多

- [官方文档](https://docs.radxa.com/e/e20c)

- [官网介绍](https://radxa.com/products#NetworkComputer)

- [瑞莎京东店铺](https://mall.jd.com/view_search-3967032-31323445-99-1-20-1.html)

- [瑞莎淘宝店铺](https://radxa.taobao.com/category-1797203497.htm?spm=a1z10.1-c-s.0.0.eaee3a483AhQEE)

## 注意

1. **不要用 root 用户进行编译**
2. 国内用户编译前最好准备好梯子
3. 默认登陆IP 192.168.1.1 密码 password

## 编译命令

1. 首先装好 Linux 系统，推荐 Debian 或 Ubuntu LTS 22/24

2. 安装编译依赖

   ```bash
   sudo apt update -y
   sudo apt full-upgrade -y
   sudo apt install -y ack antlr3 asciidoc autoconf automake autopoint binutils bison build-essential \
   bzip2 ccache clang cmake cpio curl device-tree-compiler flex gawk gcc-multilib g++-multilib gettext \
   genisoimage git gperf haveged help2man intltool libc6-dev-i386 libelf-dev libfuse-dev libglib2.0-dev \
   libgmp3-dev libltdl-dev libmpc-dev libmpfr-dev libncurses5-dev libncursesw5-dev libpython3-dev \
   libreadline-dev libssl-dev libtool llvm lrzsz libnsl-dev ninja-build p7zip p7zip-full patch pkgconf \
   python3 python3-pyelftools python3-setuptools qemu-utils rsync scons squashfs-tools subversion \
   swig texinfo uglifyjs upx-ucl unzip vim wget xmlto xxd zlib1g-dev
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

本套代码保证肯定可以编译成功。里面包括了 R24 所有源代码，包括 IPK 的。

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
rm -rf .config
make menuconfig
make V=s -j$(nproc)
```

编译完成后输出路径：bin/targets

### 使用 WSL/WSL2 进行编译

由于 WSL 的 PATH 中包含带有空格的 Windows 路径，有可能会导致编译失败，请在 `make` 前面加上：

```bash
PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
```

由于默认情况下，装载到 WSL 发行版的 NTFS 格式的驱动器将不区分大小写，因此大概率在 WSL/WSL2 的编译检查中会返回以下错误：

```txt
Build dependency: OpenWrt can only be built on a case-sensitive filesystem
```

一个比较简洁的解决方法是，在 `git clone` 前先创建 Repository 目录，并为其启用大小写敏感：

```powershell
# 以管理员身份打开终端
PS > fsutil.exe file setCaseSensitiveInfo <your_local_lede_path> enable
# 将本项目 git clone 到开启了大小写敏感的目录 <your_local_lede_path> 中
PS > git clone https://github.com/coolsnowwolf/lede <your_local_lede_path>
```

> 对已经 `git clone` 完成的项目目录执行 `fsutil.exe` 命令无法生效，大小写敏感只对新增的文件变更有效。

### macOS 原生系统进行编译

1. 在 AppStore 中安装 Xcode

2. 安装 Homebrew：

   ```bash
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
   ```

3. 使用 Homebrew 安装工具链、依赖与基础软件包：

   ```bash
   brew unlink awk
   brew install coreutils diffutils findutils gawk gnu-getopt gnu-tar grep make ncurses pkg-config wget quilt xz
   brew install gcc@11
   ```

4. 然后输入以下命令，添加到系统环境变量中：

   - intel 芯片的 mac

   ```bash
   echo 'export PATH="/usr/local/opt/coreutils/libexec/gnubin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/usr/local/opt/findutils/libexec/gnubin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/usr/local/opt/gnu-getopt/bin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/usr/local/opt/gnu-tar/libexec/gnubin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/usr/local/opt/grep/libexec/gnubin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/usr/local/opt/gnu-sed/libexec/gnubin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/usr/local/opt/make/libexec/gnubin:$PATH"' >> ~/.bashrc
   ```

   - apple 芯片的 mac

   ```zsh
   echo 'export PATH="/opt/homebrew/opt/coreutils/libexec/gnubin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/opt/homebrew/opt/findutils/libexec/gnubin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/opt/homebrew/opt/gnu-getopt/bin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/opt/homebrew/opt/gnu-tar/libexec/gnubin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/opt/homebrew/opt/grep/libexec/gnubin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/opt/homebrew/opt/gnu-sed/libexec/gnubin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/opt/homebrew/opt/make/libexec/gnubin:$PATH"' >> ~/.bashrc
   ```

5. 重新加载一下 shell 启动文件 `source ~/.bashrc`，然后输入 `bash` 进入 bash shell，就可以和 Linux 一样正常编译了

## 特别提示

1. 源代码中绝不含任何后门和可以监控或者劫持你的 HTTPS 的闭源软件， SSL 安全是互联网最后的壁垒，安全干净才是固件应该做到的。

2. 想学习 OpenWrt 开发，但是摸不着门道？自学没毅力？基础太差？怕太难学不会？跟着佐大学 OpenWrt 开发入门培训班助你能学有所成
报名地址：[点击报名](http://forgotfun.org/2018/04/openwrt-training-2018.html "报名")

3. QCA IPQ60xx 开源仓库地址：<https://github.com/coolsnowwolf/openwrt-gl-ax1800>

4. 存档版本仓库地址：<https://github.com/coolsnowwolf/openwrt>

## 捐贈

如果你觉得此项目对你有帮助，可以捐助我们，以鼓励项目能持续发展，更加完善

 ![star](doc/star.png)
