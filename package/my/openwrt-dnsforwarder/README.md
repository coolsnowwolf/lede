# openwrt-dnsforwarder
为OpenWRT路由器编译运行[dnsforwarder][A]编写的Makefile和luci界面



简介
---

本软件包为OpenWRT路由器使用dnsforwarder配置企业级DNS服务器并与dnsmasq配合进行智能黑名单解析的工具

1、支持在Web界面编辑dnsforwarder配置文件

2、支持在Web界面编辑GFWList名单和自定义的黑白名单

3、可以控制iptables对客户端发来的DNS请求进行劫持

4、与DNSMasq配合进行给予黑白名单的DNS请求转发

5、与DNSMasq配合将黑白名单中的域名解析完成后添加到ipset，为SSR路由提供依据

6、支持一键升级GFWList黑名单



依赖
---

显式依赖 `wget`，用于加载网络上的host文件


配置
---

配置文件地址 "/etc/dnsforwarer/dnsfrowarder.conf"

GFWList地址 "/etc/dnsforwarer/base-gfwlist.txt"

用户自定义黑名单 “/etc/dnsforwarer/userlist”

用户自定义白名单 “etc/dnsforwarer/whitelist”


编译
---

从 OpenWrt 的 [SDK][openwrt-sdk] 编译
```bash
# 解压下载好的 SDK
tar xjf OpenWrt-SDK-ar71xx-for-linux-x86_64-gcc-4.8-linaro_uClibc-0.9.33.2.tar.bz2
cd OpenWrt-SDK-ar71xx-*
# Clone 项目
git clone https://github.com/AlexZhuo/openwrt-dnsforwarder/ package/dnsforwarder
# 编译 po2lmo (如果有po2lmo可跳过)
pushd package/dnsforwarder/tools/po2lmo
make && sudo make install
popd
# 选择要编译的包 Network -> dnsforwarder
make menuconfig
# 开始编译
make package/dnsforwarder/compile V=99
```

截图
---
![demo](https://github.com/AlexZhuo/openwrt-dnsforwarder/raw/master/screencapture-192-168-167-101-cgi-bin-luci-admin-services-dnsforwarder-1509669292068.png)

![demo](https://github.com/AlexZhuo/openwrt-dnsforwarder/raw/master/screencapture-192-168-167-102-cgi-bin-luci-admin-services-dnsforwarder-gfwlist-1509575828363.png)

![demo](https://github.com/AlexZhuo/openwrt-dnsforwarder/raw/master/screencapture-192-168-167-101-cgi-bin-luci-admin-services-dnsforwarder-log-1509669318124.png)




[A]: https://github.com/holmium/dnsforwarder
[openwrt-sdk]: https://wiki.openwrt.org/doc/howto/obtain.firmware.sdk
