# luci-app-pdnsd
为OpenWRT路由器Pdnsd设计的luci页面和配置文件，用于使用TCP协议请求DNS以方便代理转发或避免投毒，已在[该固件][A]中使用

简介
---

本软件包为OpenWRT路由器使用Pdnsd进行TCP协议的DNS请求提供了修改配置文件的luci页面并在应用之后自动配置dnsmasq并重启Pdnsd

默认显示的配置文件：    /etc/pdnsd_gfw.cfg

默认监听端口：         `0.0.0.0:5053` 

默认模式：             tcp_only

默认上游DNS服务器：     `114.114.114.114`

默认备用服务器：        `OpenDNS`

默认备用服务器DNS端口    `53` 

该配置文件无需修改即可防止DNS污染。点击【保存&应用】后会将此配置文件覆盖"/etc/pdnsd.conf"并重启pdnsd，同时会后台自动修改【DHCP/DNS】的【DNS转发】为【127.0.0.1#5053】并勾选【忽略解析文件】并重启dnsmasq，刷新Pdnsd界面会自动检查这个配置，如果没有配置成功会以红色字提示。在取消启用后会自动恢复【DHCP/DNS】的配置。但是需要你手动去点击【DHCP/DNS】的【保存&应用】否则重启会失效并红字提示。
使用效果和更多使用方法请[参考这里][A]


依赖
---

显式依赖 `pdnsd`，支持开机自启,但是重启请注意【DHCP/DNS】中的【DNS转发】有没有成功配置成【127.0.0.1#5053】并【忽略解析文件】

配置
---

进入luci页面默认读取的是 "/etc/pdnsd_gfw.cfg" ,点击保存&应用后会以此文件覆盖"/etc/pdnsd.conf"

编译
---

从 OpenWrt 的 [SDK][openwrt-sdk] 编译
```bash
# 解压下载好的 SDK
tar xjf OpenWrt-SDK-ar71xx-for-linux-x86_64-gcc-4.8-linaro_uClibc-0.9.33.2.tar.bz2
cd OpenWrt-SDK-ar71xx-*
# Clone 项目
git clone https://github.com/AlexZhuo/luci-app-pdnsd package/luci-app-pdnsd
# 编译 po2lmo (如果有po2lmo可跳过)
pushd package/luci-app-pdnsd/tools/po2lmo
make && sudo make install
popd
# 选择要编译的包 Utilities -> LuCI -> luci-app-pdnsd
make menuconfig
# 开始编译
make package/luci-app-pdnsd/compile V=99
```

截图
---

![demo](https://github.com/AlexZhuo/BreakwallOpenWrt/raw/master/screenshots/pdnsd.png)
[A]: http://www.right.com.cn/forum/thread-198649-1-1.html
[openwrt-sdk]: https://wiki.openwrt.org/doc/howto/obtain.firmware.sdk
