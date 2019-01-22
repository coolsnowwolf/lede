OpenWrt-dist LuCI
===

 [![Download][B]][2]

APP 列表

 Name                      | Depends                  | Description
 --------------------------|--------------------------|-------------------------
 luci-app-chinadns         | [openwrt-chinadns][5]    | LuCI Support for ChinaDNS
 luci-app-dns-forwarder    | [openwrt-dns-forwarder][6]| LuCI Support for Dns-Forwarder
 luci-app-redsocks2        | [openwrt-redsocks2][R]   | LuCI Support for RedSocks2
 luci-app-shadowvpn        | [openwrt-shadowvpn][8]   | LuCI Support for ShadowVPN

编译说明
---
 > 从 OpenWrt 的 [SDK][S] 编译  

```bash
# 解压下载好的 SDK
tar xjf OpenWrt-SDK-ar71xx-for-linux-x86_64-gcc-4.8-linaro_uClibc-0.9.33.2.tar.bz2
cd OpenWrt-SDK-ar71xx-*
# Clone 项目
git clone https://github.com/aa65535/openwrt-dist-luci.git package/openwrt-dist-luci
# 编译 po2lmo (如果有po2lmo可跳过)
pushd package/openwrt-dist-luci/tools/po2lmo
make && sudo make install
popd
# 选择要编译的包 LuCI -> 3. Applications
make menuconfig
# 开始编译
make package/openwrt-dist-luci/compile V=99
```


 [B]: https://img.shields.io/github/release/aa65535/openwrt-dist-luci.svg
 [2]: https://github.com/aa65535/openwrt-dist-luci/releases/latest
 [5]: https://github.com/aa65535/openwrt-chinadns
 [6]: https://github.com/aa65535/openwrt-dns-forwarder
 [8]: https://github.com/aa65535/openwrt-shadowvpn
 [R]: https://github.com/aa65535/openwrt-redsocks2
 [S]: https://wiki.openwrt.org/doc/howto/obtain.firmware.sdk
