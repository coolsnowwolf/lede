[DNSCrypt Proxy Version2][DNSCRYPTV2] LuCI support for OpenWrt
===

Helping to setup dnscrypt-proxy2 DNS resolvers

Fun
---

- Fully configure options for client
- Support resolver list cache mode

Todo
---

- [self-sign][SIGN] for resolvers list.
- DoH server configures.

compile
---

 - Download [SDK][S], and it's depends:
   ```bash
   sudo apt-get install gawk libncurses5-dev libz-dev zlib1g-dev git ccache
   ```
 
 - Download your own SDK

   ```bash
   # Untar ar71xx platform
   tar xjf OpenWrt-SDK-15.05-ar71xx-generic_gcc-4.8-linaro_uClibc-0.9.33.2.Linux-x86_64.tar.bz2
   cd OpenWrt-SDK-*
   # update feeds
   ./scripts/feeds update packages
   # Clone
   git clone https://github.com/peter-tank/luci-app-dnscrypt-proxy2.git package/luci-app-dnscrypt-proxy2
   # select this package
   make menuconfig
   # Compile
    make package/luci-app-dnscrypt-proxy2/compile V=99
   ```
installing
---

- Depends: md5sum wget tar

  [DNSCRYPTV2]: https://github.com/DNSCrypt/dnscrypt-proxy
  [SIGN]: https://github.com/peter-tank/openwrt-minisign
  [S]: https://wiki.openwrt.org/doc/howto/obtain.firmware.sdk
