ShadowVPN for OpenWrt
===

简介
---

 本项目是 [ShadowVPN][1] 在 OpenWrt 上的移植  
 当前版本: 0.2.0-1  
 [预编译 IPK 下载][2]  

编译
---

 - 从 OpenWrt 的 [SDK][S] 编译  

   ```bash
   # 以 ar71xx 平台为例
   tar xjf OpenWrt-SDK-ar71xx-for-linux-x86_64-gcc-4.8-linaro_uClibc-0.9.33.2.tar.bz2
   cd OpenWrt-SDK-ar71xx-*
   # 获取 Makefile
   git clone https://github.com/aa65535/openwrt-shadowvpn.git package/shadowvpn
   # 选择要编译的包 Network -> ShadowVPN
   make menuconfig
   # 开始编译
   make package/shadowvpn/compile V=99
   ```

配置
---

 - 多用户配置参考 [Wiki][W]  

 - 建议[搭配 ChinaDNS-C 使用][8], 以获得更好的使用体验  

 - 使用 **路由控制** 控制流量是否走 VPN

   > 默认不带路由文件, 可读取 ChinaDNS-C 的 [chnroute][3] 生成国内路由规则, 否则为全局模式.  

 - 关于 [LuCI][L] 中路由表设置(uci: route_mode)  

    * **全局模式(0):** 启动后全部流量都经过 VPN  

    * **国内路由(1):** 路由文件中的 IP 将不经过 VPN, 如路由文件不存在则仍为**全局模式**  

    * **国外路由(2):** 只有路由文件中的 IP 经过 VPN, 如路由文件不存在则不使用 VPN  

----------

 Name                     | Description
 -------------------------|-----------------------------------
 [openwrt-chinadns][5]    | ChinaDNS-C for OpenWrt
 [openwrt-dnsmasq][6]     | Dnsmasq Patch & Makefile for OpenWrt
 [openwrt-shadowsocks][7] | Shadowsocks-libev for OpenWrt
 [openwrt-redsocks2][R]   | RedSocks2 for OpenWrt
 [openwrt-dist-luci][L]   | LuCI Applications for OpenWrt-dist


  [1]: https://github.com/clowwindy/ShadowVPN
  [2]: https://sourceforge.net/projects/openwrt-dist/files/shadowvpn/
  [3]: https://github.com/clowwindy/ChinaDNS-C/blob/master/chnroute.txt
  [5]: https://github.com/aa65535/openwrt-chinadns
  [6]: https://github.com/aa65535/openwrt-dnsmasq
  [7]: https://github.com/shadowsocks/openwrt-shadowsocks
  [8]: https://sourceforge.net/p/openwrt-dist/wiki/Plan6/
  [R]: https://github.com/aa65535/openwrt-redsocks2
  [S]: http://wiki.openwrt.org/doc/howto/obtain.firmware.sdk
  [L]: https://github.com/aa65535/openwrt-dist-luci
  [W]: https://github.com/clowwindy/ShadowVPN/wiki/Configure-Multiple-Users
