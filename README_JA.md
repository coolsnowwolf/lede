# OpenWrt とパッケージの Lean の git ソースへようこそ

I18N: [English](README_EN.md) | [简体中文](README.md) | [日本語](README_JA.md)

## 公式チャンネル
<!--
如有技术问题需要讨论或者交流，欢迎加入以下群：
1. QQ 讨论群： Op固件技术研究群 ,号码 891659613 ，加群链接：[点击加入](https://jq.qq.com/?_wv=1027&k=XL8SK5aC "Op固件技术研究群")
2. TG 讨论群： OP 编译官方大群 ，加群链接：[点击加入](https://t.me/JhKgAA6Hx1 "OP 编译官方大群")
3. Rockchip RK3568 预编译固件发布 Release 下载更新地址 (包括 H68K )：<https://github.com/coolsnowwolf/lede/releases/tag/20220716>
-->

ディスカッションや共有したい技術的な質問がある場合は、以下のチャンネルにお気軽にご参加ください:
1. QQ グループ: *OpenWRT ファームウェア技術研究グループ*、グループ番号は `891659613` です。グループに参加する: [リンク](https://jq.qq.com/?_wv=1027&k=XL8SK5aC "Op固件技术研究群")。
    -  [クリックして QQ クライアントをダウンロードする](https://im.qq.com/pcqq)。
2. Telegram グループ: *OpenWRT ファームウェア技術研究グループ*。グループに参加する: [リンク](https://t.me/JhKgAA6Hx1 "OP 编译官方大群")。
3. Rockchip **RK3568** コンパイル済みファームウェアリリース (H68K を含む): [クリックしてダウンロード](https://github.com/coolsnowwolf/lede/releases/tag/20220716)。

<div align="left">
    <a href="https://item.taobao.com/item.htm?spm=a230r.1.14.11.4bb55247rdHEAP&id=702787603594&ns=1&abbucket=17#detail
">
        <img style="margin: 0px 0px 0px 40px;" src="https://github.com/coolsnowwolf/lede/blob/master/doc/h68k.jpg?raw=true" width=600  />
    </a>
</div>

4. Rockchip **RK3588**コンパイル済みファームウェアリリース（H68K を含む）: [クリックしてダウンロード](https://github.com/coolsnowwolf/lede/releases/tag/20230609)。


## 注意
<!--
1. **不要用 root 用户进行编译**
2. 国内用户编译前最好准备好梯子
3. 默认登陆IP 192.168.1.1 密码 password
-->
1. **OpenWRT を決して `root` としてコンパイルしないこと**
2. 中国本土にお住まいの方は、ぜひ **REAL** インターネットをご覧ください。
3. デフォルトのログイン IP は `192.168.1.1` で、パスワードは `password` です。

## コンパイル方法
<!--
1. 首先装好 Linux 系统，推荐 Debian 11 或 Ubuntu LTS
2. 安装编译依赖

   ```bash
   sudo apt update -y
   sudo apt full-upgrade -y
   sudo apt install -y ack antlr3 asciidoc autoconf automake autopoint binutils bison build-essential \
   bzip2 ccache cmake cpio curl device-tree-compiler fastjar flex gawk gettext gcc-multilib g++-multilib \
   git gperf haveged help2man intltool libc6-dev-i386 libelf-dev libglib2.0-dev libgmp3-dev libltdl-dev \
   libmpc-dev libmpfr-dev libncurses5-dev libncursesw5-dev libreadline-dev libssl-dev libtool lrzsz \
   mkisofs msmtp nano ninja-build p7zip p7zip-full patch pkgconf python2.7 python3 python3-pyelftools \
   libpython3-dev qemu-utils rsync scons squashfs-tools subversion swig texinfo uglifyjs upx-ucl unzip \
   vim wget xmlto xxd zlib1g-dev python3-setuptools
   ```
-->

1. Linuxディストリビューションをインストールし、Debian 11 または Ubuntu LTS を推奨します。
2. 依存関係をインストールする:

   ```bash
   sudo apt update -y
   sudo apt full-upgrade -y
   sudo apt install -y ack antlr3 asciidoc autoconf automake autopoint binutils bison build-essential \
   bzip2 ccache cmake cpio curl device-tree-compiler fastjar flex gawk gettext gcc-multilib g++-multilib \
   git gperf haveged help2man intltool libc6-dev-i386 libelf-dev libglib2.0-dev libgmp3-dev libltdl-dev \
   libmpc-dev libmpfr-dev libncurses5-dev libncursesw5-dev libreadline-dev libssl-dev libtool lrzsz \
   mkisofs msmtp nano ninja-build p7zip p7zip-full patch pkgconf python2.7 python3 python3-pyelftools \
   libpython3-dev qemu-utils rsync scons squashfs-tools subversion swig texinfo uglifyjs upx-ucl unzip \
   vim wget xmlto xxd zlib1g-dev python3-setuptools
   ```

<!--
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
-->

3. ソースコードをクローンし、`feeds` を更新し、設定する:

   ```bash
   git clone https://github.com/coolsnowwolf/lede
   cd lede
   ./scripts/feeds update -a
   ./scripts/feeds install -a
   make menuconfig
   ```

4. ライブラリのダウンロードとファームウェアのコンパイル
   > (`-j` はスレッドカウント、最初のビルドはシングルスレッドを推奨):

   ```bash
   make -j8 download V=s
   make -j1 V=s
   ```

<!--
本套代码保证肯定可以编译成功。里面包括了 R23 所有源代码，包括 IPK 的。

你可以自由使用，但源码编译二次发布请注明我的 GitHub 仓库链接。谢谢合作！
-->

これらのコマンドは、ソースコードを正常にコンパイルするためのものです。
R23 のソースコードは IPK を含めてすべて含まれています。

このソースコードはご自由にお使いいただけますが、再配布の際はこの GitHub リポジトリをリンクしてください。
ご協力ありがとうございました！

<!--
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
-->

リビルド:

```bash
cd lede
git pull
./scripts/feeds update -a
./scripts/feeds install -a
make defconfig
make download -j8
make V=s -j$(nproc)
```

再設定が必要な場合:

```bash
rm -rf ./tmp && rm -rf .config
make menuconfig
make V=s -j$(nproc)
```

ビルドの成果物は `bin/targets` ディレクトリに出力されます。

### WSL/WSL2 をビルド環境として使用している場合

<!--
由于 WSL 的 PATH 中包含带有空格的 Windows 路径，有可能会导致编译失败，请在 `make` 前面加上：

```bash
PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
```
-->
WSL の `PATH` には、Windows のパスが空白で含まれている可能性があり、コンパイルに失敗することがあります。コンパイルする前に、ローカルの環境プロファイルに以下の行を追加してください:

```bash
# 例えば、~/.bashrc などのプロファイルを更新した後、再読み込みを行う。
cat << EOF >> ~/.bashrc
export PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:$PATH"
EOF
source ~/.bashrc
```

<!--
由于默认情况下，装载到 WSL 发行版的 NTFS 格式的驱动器将不区分大小写，因此大概率在 WSL/WSL2 的编译检查中会返回以下错误：

```txt
Build dependency: OpenWrt can only be built on a case-sensitive filesystem
```

一个比较简洁的解决方法是，在 `git clone` 前先创建 Repository 目录，并为其启用大小写敏感：

```powershell
# 以管理员身份打开终端
PS > fsutil.exe file setCaseSensitiveInfo <your_local_lede_path> enable
# 将本项目 git clone 到开启了大小写敏感的目录 <your_local_lede_path> 中
PS > git clone git@github.com:coolsnowwolf/lede.git <your_local_lede_path>
```

> 对已经 `git clone` 完成的项目目录执行 `fsutil.exe` 命令无法生效，大小写敏感只对新增的文件变更有效。
-->

WSL ディストリビューションにマウントされた NTFS フォーマットのドライブは、デフォルトで大文字と小文字が区別されません。このため、WSL/WSL2 でコンパイルすると、次のようなエラーが発生します:

```txt
Build dependency: OpenWrt can only be built on a case-sensitive filesystem
```

単純な解決策は、`git clone` の前に大文字小文字を区別してリポジトリ用のディレクトリを作成することです:

```powershell
# 管理者としてターミナルを開く
PS > fsutil.exe file setCaseSensitiveInfo <your_local_lede_path> enable
# 大文字と小文字を区別して、このリポジトリを <your_local_lede_path> ディレクトリにクローンする
PS > git clone git@github.com:coolsnowwolf/lede.git <your_local_lede_path>
```

> すでに `git clone` されたディレクトリでは、`fsutil.exe` は有効になりません。大文字小文字の区別はディレクトリの新しい変更に対してのみ有効になります。

### macOS コンパイル
<!--
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
-->

1. AppStore から Xcode をインストールする
2. Homebrew をインストールする:
   ```bash
   /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
   ```
3. Homebrew でツールチェーン、依存関係、パッケージをインストールする:
   ```bash
   brew unlink awk
   brew install coreutils diffutils findutils gawk gnu-getopt gnu-tar grep make ncurses pkg-config wget quilt xz
   brew install gcc@11
   ```
4. システム環境のアップデート:

   -  MacのIntelシリコンバージョン
   ```bash
   echo 'export PATH="/usr/local/opt/coreutils/libexec/gnubin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/usr/local/opt/findutils/libexec/gnubin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/usr/local/opt/gnu-getopt/bin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/usr/local/opt/gnu-tar/libexec/gnubin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/usr/local/opt/grep/libexec/gnubin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/usr/local/opt/gnu-sed/libexec/gnubin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/usr/local/opt/make/libexec/gnubin:$PATH"' >> ~/.bashrc
   ```

   -  MacのAppleシリコンバージョン
   
   ```zsh
   echo 'export PATH="/opt/homebrew/opt/coreutils/libexec/gnubin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/opt/homebrew/opt/findutils/libexec/gnubin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/opt/homebrew/opt/gnu-getopt/bin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/opt/homebrew/opt/gnu-tar/libexec/gnubin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/opt/homebrew/opt/grep/libexec/gnubin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/opt/homebrew/opt/gnu-sed/libexec/gnubin:$PATH"' >> ~/.bashrc
   echo 'export PATH="/opt/homebrew/opt/make/libexec/gnubin:$PATH"' >> ~/.bashrc
   ```
5. シェルプロファイル `source ~/.bashrc && bash` を再読み込みすれば、Linux のように普通にコンパイルできます。

## 宣言
<!--
1. 源代码中绝不含任何后门和可以监控或者劫持你的 HTTPS 的闭源软件， SSL 安全是互联网最后的壁垒。安全干净才是固件应该做到的；

2. 想学习 OpenWrt 开发，但是摸不着门道？自学没毅力？基础太差？怕太难学不会？跟着佐大学 OpenWrt 开发入门培训班助你能学有所成
报名地址：[点击报名](http://forgotfun.org/2018/04/openwrt-training-2018.html "报名")

3. QCA IPQ60xx 开源仓库地址：<https://github.com/coolsnowwolf/openwrt-gl-ax1800>

4. 存档版本仓库地址：<https://github.com/coolsnowwolf/openwrt>
-->
1. このソースコードには、HTTPS トラフィックを監視／キャプチャできるバックドアやクローズドソースアプリケーションは含まれていません。SSL セキュリティはサイバーセキュリティの最後の城です。安全性はファームウェアがすべきことです。
2. OpenWRT の開発を学びたいが、何から始めたらいいかわからないですか？自己学習のモチベーションが上がらない？基礎知識が足りない？ズオ氏の初心者 OpenWRT トレーニングコースで一緒に OpenWRT 開発を学びましょう。お申し込みは[こちら](http://forgotfun.org/2018/04/openwrt-training-2018.html)をクリックしてください。
3. QCA IPQ60xx オープンソースリポジトリ: <https://github.com/coolsnowwolf/openwrt-gl-ax1800>
4. OpenWRT アーカイブリポジトリ: <https://github.com/coolsnowwolf/openwrt>

## ソフトウェアルーター入門
<!--
硬酷R2 - N95/N300迷你四网HomeLab服务器

(商品介绍页面 - 硬酷科技（支持花呗）)：

[预售链接](https://item.taobao.com/item.htm?ft=t&id=719159813003)

[![r1](doc/r1.jpg)](https://item.taobao.com/item.htm?ft=t&id=719159813003)
-->
Yingku R2 - N95/N300 Mini Four-Network HomeLab Server

(紹介ページ - Yingku Technology (support AliPay Huabei)):

[先行販売リンク](https://item.taobao.com/item.htm?ft=t&id=719159813003)
<div align="left">
<a href="https://item.taobao.com/item.htm?ft=t&id=719159813003">
  <img src="doc/r1.jpg" width = "600" alt="" align=center />
</a>
</div>
<br>

## 寄付

<!--
如果你觉得此项目对你有帮助，可以捐助我们，以鼓励项目能持续发展，更加完善

 ![star](doc/star.png)
-->
このプロジェクトがあなたのお役に立てたのであれば、このプロジェクトの発展を支援するための寄付をご検討ください。

<div align="left">
  <img src="./doc/star.png" width = "400" alt="" align=center />
</div>
<br>
