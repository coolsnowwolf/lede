<h2 align="center">
 <img src="https://cdn.jsdelivr.net/gh/Dreamacro/clash/docs/logo.png" alt="Clash" width="200">
  <br>Luci App For Clash <br>
</h2>

  <p align="center">
	A rule based custom proxy client for Openwrt based on <a href="https://github.com/Dreamacro/clash" target="_blank">Clash</a>.
  </p>
  <p align="center">
  <a target="_blank" href="https://github.com/frainzy1477/luci-app-clash/releases/tag/v1.7.5.7">
    <img src="https://img.shields.io/badge/luci%20for%20clash-v1.7.5.7-blue.svg"> 	  
  </a>
  <a href="https://github.com/frainzy1477/luci-app-clash/releases" target="_blank">
        <img src="https://img.shields.io/github/downloads/frainzy1477/luci-app-clash/total.svg?style=flat-square"/>
   </a>
  </p>

  
 ## Install
- Upload ipk file to tmp folder
- cd /tmp
- opkg update
- opkg install luci-app-clash_v1.7.5.7_all.ipk  
- opkg install luci-app-clash_v1.7.5.7_all.ipk --force-depends

 ## Uninstall
- opkg remove luci-app-clash 
- opkg remove luci-app-clash --force-remove

## Features
- Subscription Config
- Config Upload
- Create Config
- GeoIP Database Update
- TProxy UDP
- IP Query
- DNS Forwarding
- Support Trojan(Trojan-GFW & Trojan-Go)
- Support SSR
- Support SS
- Support V2ray
- Support Snell
- Ping Custom Proxy Servers
- Tun Support
- Access Control
- Support Provider,
- Game Rules 
- Restore Config

## Dependency

- bash
- coreutils
- coreutils-nohup
- coreutils-base64
- ipset
- iptables
- luci
- luci-base
- wget
- libustream-openssl 
- curl
- jsonfilter
- ca-certificates
- iptables-mod-tproxy
- kmod-tun


## compile
---

 - Download [SDK](https://wiki.openwrt.org/doc/howto/obtain.firmware.sdk), and it's depends:
   ```bash
   sudo apt-get install gawk libncurses5-dev libz-dev zlib1g-dev  git ccache
   ```
 
 - Download your own SDK

   ```bash
   # Untar ar71xx platform
   tar xjf OpenWrt-SDK-15.05-ar71xx-generic_gcc-4.8-linaro_uClibc-0.9.33.2.Linux-x86_64.tar.bz2
   cd OpenWrt-SDK-*
   # update feeds
   ./scripts/feeds update packages
   # Clone
   git clone https://github.com/frainzy1477/luci-app-clash.git package/luci-app-clash
   # select this package
   make menuconfig
   
   # Compile and install po2lmo bin for build i18n language files
   pushd package/luci-app-clash/tools/po2lmo
   make && sudo make install
   popd
   # I18n language files
   po2lmo ./package/luci-app-clash/po/zh-cn/clash.po ./package/luci-app-clash/po/zh-cn/clash.zh-cn.lmo
   
   # Compile
    make package/luci-app-clash/compile V=99
   ```
   
## License  
Luci App For Clash - OpenWrt is released under the GPL v3.0 License - see detailed [LICENSE](https://github.com/frainzy1477/luci-app-clash/blob/master/LICENSE) .


