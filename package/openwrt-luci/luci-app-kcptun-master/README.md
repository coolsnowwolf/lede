# luci-app-kcptun

Luci support for kcptun

OpenWrt/LEDE 上的 Kcptun Luci 支持界面

[![Release Version](https://img.shields.io/github/release/kuoruan/luci-app-kcptun.svg)](https://github.com/kuoruan/luci-app-kcptun/releases/latest) [![Latest Release Download](https://img.shields.io/github/downloads/kuoruan/luci-app-kcptun/latest/total.svg)](https://github.com/kuoruan/luci-app-kcptun/releases/latest)

## 安装说明

1. 到 [release](https://github.com/kuoruan/luci-app-kcptun/releases) 页面下载最新版 luci-app-kcptun 和 luci-i18n-kcptun-zh-cn (简体中文翻译文件)
2. 将下载好的 ipk 文件上传到路由器任意目录下, 如 /tmp
3. 先安装 luci-app-kcptun 再安装 luci-i18n-kcptun-zh-cn

```sh
opkg install luci-app-kcptun_*.ipk
opkg install luci-i18n-kcptun-zh-cn_*.ipk
```

安装好 LuCI 之后，进入配置页面，会提示 ```客户端文件配置有误```。

若路由器上已经有 Kcptun 客户端，直接配置好路径即可；如果没有，请手动下载后上传到路由器。

## 客户端文件下载

1. OpenWrt 可用的 Kcptun：[https://github.com/kuoruan/openwrt-kcptun](https://github.com/kuoruan/openwrt-kcptun)

2. 官方版本: [https://github.com/xtaci/kcptun/releases](https://github.com/xtaci/kcptun/releases)

## 编译说明

下载 OpenWrt SDK 或者完整源码，进入根目录，运行命令下载 ```luci-app-kcptun``` 源码

```sh
git clone https://github.com/kuoruan/luci-app-kcptun.git package/luci-app-kcptun
```

然后按照正常的编译流程：

```sh
./scripts/feeds update luci-app-kcptun
./scripts/feeds install luci-app-kcptun

make menuconfig
make
```

## 卸载说明

卸载时需要先卸载 luci-i18n-kcptun-zh-cn, 再卸载 luci-app-kcptun

```sh
opkg remove luci-i18n-kcptun-zh-cn
opkg remove luci-app-kcptun
```
