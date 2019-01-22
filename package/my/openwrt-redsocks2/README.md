RedSocks2 for OpenWrt
===

简介
---

 本项目是 [RedSocks2][1] 在 OpenWrt 上的移植  
 当前版本: 0.60-2  
 [预编译 IPK 下载][2]  

编译
---

 - 从 OpenWrt 的 [SDK][S] 编译  

   ```bash
   # 以 ar71xx 平台为例
   tar xjf OpenWrt-SDK-ar71xx-for-linux-x86_64-gcc-4.8-linaro_uClibc-0.9.33.2.tar.bz2
   cd OpenWrt-SDK-ar71xx-*
   # 获取 Makefile
   git clone https://github.com/aa65535/openwrt-redsocks2.git package/redsocks2
   # 选择要编译的包 Network -> redsocks2
   make menuconfig
   # 开始编译
   make package/redsocks2/compile V=99
   ```

----------

 Name                     | Description
 -------------------------|-----------------------------------
 [openwrt-chinadns][5]    | ChinaDNS-C for OpenWrt
 [openwrt-dnsmasq][6]     | Dnsmasq Patch & Makefile for OpenWrt
 [openwrt-shadowsocks][7] | Shadowsocks-libev for OpenWrt
 [openwrt-shadowvpn][8]   | ShadowVPN for OpenWrt
 [openwrt-dist-luci][L]   | LuCI Applications for OpenWrt-dist


  [1]: https://github.com/semigodking/redsocks
  [2]: http://sourceforge.net/projects/openwrt-dist/files/redsocks2/
  [5]: https://github.com/aa65535/openwrt-chinadns
  [6]: https://github.com/aa65535/openwrt-dnsmasq
  [7]: https://github.com/shadowsocks/openwrt-shadowsocks
  [8]: https://github.com/aa65535/openwrt-shadowvpn
  [S]: http://wiki.openwrt.org/doc/howto/obtain.firmware.sdk
  [L]: https://github.com/aa65535/openwrt-dist-luci
