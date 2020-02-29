#
# This is free software, lisence use MIT.
# 
# Copyright (C) 2019 P3TERX <https://p3terx.com>
# Copyright (C) 2019 KFERMercer <KFER.Mercer@gmail.com>
# 
# <https://github.com/KFERMercer/OpenWrt-CI>
#

name: 云编译X86 
on:
  push:
    branches: 
      - master     
      #     ↑↑↑ 改成你的分支名, 一般是master
       
  schedule: 
    - cron: 30 0 * * * 
    #     ↑↑↑ 设置开始编译时间。格式为UTC，即北京时间减去8小时……，如：0 0 * * 1，为北京时间每周一的早上8点开始编译……
       
  # release:
  #   types: [published]

jobs:
  build:
    runs-on: ubuntu-latest

    steps:
      - name: 检测-Checkout
        uses: actions/checkout@master
        with:
          ref: master
        #      ↑↑↑ 改成你的分支名, 一般是master

      - name: 初始化环境-Initialization environment
        env:
          DEBIAN_FRONTEND: noninteractive
        run: |
          docker rmi `docker images -q`
          echo "Deleting files, please wait ..."
          sudo rm -rf \
            /usr/share/dotnet \
            /etc/mysql \
            /etc/php
          sudo -E apt-get -y purge \
            azure-cli \
            ghc* \
            zulu* \
            hhvm \
            llvm* \
            firefox \
            google* \
            dotnet* \
            powershell \
            openjdk* \
            mysql* \
            php*
          sudo -E apt-get update
          sudo -E apt-get -y install build-essential asciidoc binutils bzip2 gawk gettext git libncurses5-dev libz-dev patch unzip zlib1g-dev lib32gcc1 libc6-dev-i386 subversion flex uglifyjs git-core gcc-multilib p7zip p7zip-full msmtp libssl-dev texinfo libglib2.0-dev xmlto qemu-utils upx libelf-dev autoconf automake libtool autopoint device-tree-compiler
          sudo -E apt-get -y autoremove --purge
          sudo -E apt-get clean
          
      - name: 增加插件或主题
        run: |
          cd package
          mkdir openwrt-packages
          cd openwrt-packages
          git clone https://github.com/openwrt-develop/luci-theme-atmaterial.git
          git clone https://github.com/rosywrt/luci-theme-rosy.git
      - name: 更新和安装源-Update feeds
        run: |
          ./scripts/feeds update -a
          ./scripts/feeds install -a
      - name: 自定义配置文件-Custom configure file
        run: |
          rm -f ./.config*
          touch ./.config
          #
          # ========================固件定制部分========================
          # 
          # 如果不对本区块做出任何编辑, 则生成默认配置固件. 
          # 
          # 以下为定制化固件选项和说明:
          #
          # 有些插件/选项是默认开启的, 如果想要关闭, 请参照以下示例进行编写:
          # 
          #          =========================================
          #         |  # 取消编译VMware镜像:                   |
          #         |  cat >> .config <<EOF                   |
          #         |  # CONFIG_VMDK_IMAGES is not set        |
          #         |  EOF                                    |
          #          =========================================
          #
          # 以下是一些提前准备好的一些插件选项.
          # 直接取消注释相应代码块即可应用. 不要取消注释代码块上的汉字说明.
          # 如果不需要代码块里的某一项配置, 只需要删除相应行.
          #
          # 如果需要其他插件, 请按照示例自行添加.
          # 注意, 只需添加依赖链顶端的包. 如果你需要插件 A, 同时 A 依赖 B, 即只需要添加 A.
          # 
          # 无论你想要对固件进行怎样的定制, 都需要且只需要修改 EOF 回环内的内容.
          # 
          # 编译x64固件:
          cat >> .config <<EOF
          CONFIG_TARGET_x86=y
          CONFIG_TARGET_x86_64=y
          CONFIG_TARGET_x86_64_Generic=y
          EOF
          # 固件压缩:
          cat >> .config <<EOF
          CONFIG_TARGET_IMAGES_GZIP=y
          EOF
          # 编译UEFI固件:
          cat >> .config <<EOF
          CONFIG_EFI_IMAGES=y
          EOF
          # IPv6支持:
          cat >> .config <<EOF
          CONFIG_PACKAGE_dnsmasq_full_dhcpv6=y
          CONFIG_PACKAGE_ipv6helper=y
          EOF
          # 多文件系统支持:
          cat >> .config <<EOF
          CONFIG_PACKAGE_kmod-fs-nfs=y
          CONFIG_PACKAGE_kmod-fs-nfs-common=y
          CONFIG_PACKAGE_kmod-fs-nfs-v3=y
          CONFIG_PACKAGE_kmod-fs-nfs-v4=y
          CONFIG_PACKAGE_kmod-fs-ntfs=y
          CONFIG_PACKAGE_kmod-fs-squashfs=y
          EOF
          # USB3.0支持:
          cat >> .config <<EOF
          CONFIG_PACKAGE_kmod-usb-ohci=y
          CONFIG_PACKAGE_kmod-usb-ohci-pci=y
          CONFIG_PACKAGE_kmod-usb2=y
          CONFIG_PACKAGE_kmod-usb2-pci=y
          CONFIG_PACKAGE_kmod-usb3=y
          EOF
          # 常用LuCI插件选择:
          cat >> .config <<EOF
          CONFIG_PACKAGE_luci-app-accesscontrol=y
          # CONFIG_PACKAGE_luci-app-acme is not set
          # CONFIG_PACKAGE_luci-app-adblock is not set
          CONFIG_PACKAGE_luci-app-adbyby-plus=y
          # CONFIG_PACKAGE_luci-app-advanced-reboot is not set
          # CONFIG_PACKAGE_luci-app-ahcp is not set
          # CONFIG_PACKAGE_luci-app-amule is not set
          # CONFIG_PACKAGE_luci-app-aria2 is not set
          CONFIG_PACKAGE_luci-app-arpbind=y
          # CONFIG_PACKAGE_luci-app-asterisk is not set
          # CONFIG_PACKAGE_luci-app-attendedsysupgrade is not set
          CONFIG_PACKAGE_luci-app-autoreboot=y
          # CONFIG_PACKAGE_luci-app-baidupcs-web is not set
          # CONFIG_PACKAGE_luci-app-bcp38 is not set
          # CONFIG_PACKAGE_luci-app-bird1-ipv4 is not set
          # CONFIG_PACKAGE_luci-app-bird1-ipv6 is not set
          # CONFIG_PACKAGE_luci-app-bmx6 is not set
          # CONFIG_PACKAGE_luci-app-bmx7 is not set
          # CONFIG_PACKAGE_luci-app-cifsd is not set
          # CONFIG_PACKAGE_luci-app-cjdns is not set
          # CONFIG_PACKAGE_luci-app-clamav is not set
          # CONFIG_PACKAGE_luci-app-commands is not set
          # CONFIG_PACKAGE_luci-app-cshark is not set
          CONFIG_PACKAGE_luci-app-ddns=y
          # CONFIG_PACKAGE_luci-app-diag-core is not set
          # CONFIG_PACKAGE_luci-app-dnscrypt-proxy is not set
          # CONFIG_PACKAGE_luci-app-dnsforwarder is not set
          # CONFIG_PACKAGE_luci-app-dockerman is not set
          # CONFIG_PACKAGE_luci-app-dump1090 is not set
          # CONFIG_PACKAGE_luci-app-dynapoint is not set
          # CONFIG_PACKAGE_luci-app-e2guardian is not set
          # CONFIG_PACKAGE_luci-app-familycloud is not set
          CONFIG_PACKAGE_luci-app-filetransfer=y
          CONFIG_PACKAGE_luci-app-firewall=y
          CONFIG_PACKAGE_luci-app-flowoffload=y
          # CONFIG_PACKAGE_luci-app-freifunk-diagnostics is not set
          # CONFIG_PACKAGE_luci-app-freifunk-policyrouting is not set
          # CONFIG_PACKAGE_luci-app-freifunk-widgets is not set
          # CONFIG_PACKAGE_luci-app-frpc is not set
          # CONFIG_PACKAGE_luci-app-fwknopd is not set
          # CONFIG_PACKAGE_luci-app-guest-wifi is not set
          # CONFIG_PACKAGE_luci-app-haproxy-tcp is not set
          # CONFIG_PACKAGE_luci-app-hd-idle is not set
          # CONFIG_PACKAGE_luci-app-hnet is not set
          CONFIG_PACKAGE_luci-app-ipsec-vpnd=y
          # CONFIG_PACKAGE_luci-app-kodexplorer is not set
          # CONFIG_PACKAGE_luci-app-lxc is not set
          # CONFIG_PACKAGE_luci-app-meshwizard is not set
          # CONFIG_PACKAGE_luci-app-minidlna is not set
          # CONFIG_PACKAGE_luci-app-mjpg-streamer is not set
          # CONFIG_PACKAGE_luci-app-mtwifi is not set
          CONFIG_PACKAGE_luci-app-mwan3=y
          # CONFIG_PACKAGE_luci-app-mwan3helper is not set
          # CONFIG_PACKAGE_luci-app-n2n_v2 is not set
          # CONFIG_PACKAGE_luci-app-netdata is not set
          # CONFIG_PACKAGE_luci-app-nft-qos is not set
          CONFIG_PACKAGE_luci-app-nlbwmon=y
          # CONFIG_PACKAGE_luci-app-noddos is not set
          # CONFIG_PACKAGE_luci-app-nps is not set
          # CONFIG_PACKAGE_luci-app-ntpc is not set
          # CONFIG_PACKAGE_luci-app-ocserv is not set
          # CONFIG_PACKAGE_luci-app-olsr is not set
          # CONFIG_PACKAGE_luci-app-olsr-services is not set
          # CONFIG_PACKAGE_luci-app-olsr-viz is not set
          # CONFIG_PACKAGE_luci-app-openvpn is not set
          # CONFIG_PACKAGE_luci-app-openvpn-server is not set
          # CONFIG_PACKAGE_luci-app-p910nd is not set
          # CONFIG_PACKAGE_luci-app-pagekitec is not set
          # CONFIG_PACKAGE_luci-app-polipo is not set
          # CONFIG_PACKAGE_luci-app-pppoe-relay is not set
          # CONFIG_PACKAGE_luci-app-pptp-server is not set
          # CONFIG_PACKAGE_luci-app-privoxy is not set
          # CONFIG_PACKAGE_luci-app-qbittorrent is not set
          # CONFIG_PACKAGE_luci-app-qos is not set
          # CONFIG_PACKAGE_luci-app-radicale is not set
          CONFIG_PACKAGE_luci-app-ramfree=y
          # CONFIG_PACKAGE_luci-app-rp-pppoe-server is not set
          CONFIG_PACKAGE_luci-app-samba=y
          # CONFIG_PACKAGE_luci-app-samba4 is not set
          # CONFIG_PACKAGE_luci-app-shadowsocks-libev is not set
          # CONFIG_PACKAGE_luci-app-shairplay is not set
          # CONFIG_PACKAGE_luci-app-siitwizard is not set
          # CONFIG_PACKAGE_luci-app-simple-adblock is not set
          # CONFIG_PACKAGE_luci-app-softethervpn is not set
          # CONFIG_PACKAGE_luci-app-splash is not set
          CONFIG_PACKAGE_luci-app-sqm=y
          # CONFIG_PACKAGE_luci-app-squid is not set
          CONFIG_PACKAGE_luci-app-ssr-plus=y
          CONFIG_PACKAGE_luci-app-ssr-plus_INCLUDE_Shadowsocks=y
          CONFIG_PACKAGE_luci-app-ssr-plus_INCLUDE_Simple_obfs=y
          CONFIG_PACKAGE_luci-app-ssr-plus_INCLUDE_V2ray_plugin=y
          CONFIG_PACKAGE_luci-app-ssr-plus_INCLUDE_V2ray=y
          CONFIG_PACKAGE_luci-app-ssr-plus_INCLUDE_Trojan=y
          # CONFIG_PACKAGE_luci-app-ssr-plus_INCLUDE_Kcptun is not set
          CONFIG_PACKAGE_luci-app-ssr-plus_INCLUDE_ShadowsocksR_Server=y
          # CONFIG_PACKAGE_luci-app-ssr-plus_INCLUDE_ShadowsocksR_Socks is not set
          # CONFIG_PACKAGE_luci-app-ssrserver-python is not set
          # CONFIG_PACKAGE_luci-app-statistics is not set
          CONFIG_PACKAGE_luci-app-syncdial=y
          # CONFIG_PACKAGE_luci-app-tinyproxy is not set
          # CONFIG_PACKAGE_luci-app-transmission is not set
          # CONFIG_PACKAGE_luci-app-travelmate is not set
          # CONFIG_PACKAGE_luci-app-ttyd is not set
          # CONFIG_PACKAGE_luci-app-udpxy is not set
          # CONFIG_PACKAGE_luci-app-uhttpd is not set
          CONFIG_PACKAGE_luci-app-unblockmusic=y
          CONFIG_PACKAGE_luci-app-unblockneteasemusic-go=y
          CONFIG_PACKAGE_luci-app-unblockneteasemusic-mini=y
          # CONFIG_PACKAGE_luci-app-unbound is not set
          CONFIG_PACKAGE_luci-app-upnp=y
          CONFIG_PACKAGE_luci-app-usb-printer=y
          CONFIG_PACKAGE_luci-app-v2ray-server=y
          CONFIG_PACKAGE_luci-app-verysync=y
          CONFIG_PACKAGE_luci-app-vlmcsd=y
          # CONFIG_PACKAGE_luci-app-vnstat is not set
          # CONFIG_PACKAGE_luci-app-vpnbypass is not set
          CONFIG_PACKAGE_luci-app-vsftpd=y
          # CONFIG_PACKAGE_luci-app-watchcat is not set
          # CONFIG_PACKAGE_luci-app-webadmin is not set
          # CONFIG_PACKAGE_luci-app-wifischedule is not set
          # CONFIG_PACKAGE_luci-app-wireguard is not set
          # CONFIG_PACKAGE_luci-app-wol is not set
          # CONFIG_PACKAGE_luci-app-wrtbwmon is not set
          # CONFIG_PACKAGE_luci-app-xlnetacc is not set
          CONFIG_PACKAGE_luci-app-zerotier=y
          EOF
          # LuCI主题:
          cat >> .config <<EOF
          # CONFIG_PACKAGE_luci-theme-argon is not set
          # CONFIG_PACKAGE_luci-theme-netgear is not set
          CONFIG_PACKAGE_luci-theme-atmaterial=y
          CONFIG_PACKAGE_luci-theme-rosy=y
          EOF
          # 常用软件包:
          cat >> .config <<EOF
          CONFIG_PACKAGE_curl=y
          CONFIG_PACKAGE_htop=y
          CONFIG_PACKAGE_nano=y
          CONFIG_PACKAGE_screen=y
          CONFIG_PACKAGE_tree=y
          CONFIG_PACKAGE_vim-fuller=y
          CONFIG_PACKAGE_wget=y
          EOF
          # 取消编译VMware镜像以及镜像填充 (不要删除被缩进的注释符号):
          cat >> .config <<EOF
          # CONFIG_TARGET_IMAGES_PAD is not set
          # CONFIG_VMDK_IMAGES is not set
          EOF
          # 
          # ========================固件定制部分结束========================
          # 
          sed -i 's/^[ \t]*//g' ./.config
          make defconfig
      - name: 下载源代码包-Download package source code
        run: |
          make download -j8
          find dl -size -1024c -exec ls -l {} \;
          find dl -size -1024c -exec rm -f {} \;
      - name: 编译固件-Compile firmware
        run: |
          echo -e "$(nproc) thread build."
          make -j$(nproc) V=s
      - name : 上传固件-Upload artifact
        uses: actions/upload-artifact@master
        with:
          name: OpenWrt firmware
          path: bin/targets
