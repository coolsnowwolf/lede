# luci-app-kcptun

Luci support for kcptun

OpenWrt/LEDE 上的 Kcptun Luci 支持界面

[![Release Version](https://img.shields.io/github/release/kuoruan/luci-app-kcptun.svg)](https://github.com/kuoruan/luci-app-kcptun/releases/latest) [![Latest Release Download](https://img.shields.io/github/downloads/kuoruan/luci-app-kcptun/latest/total.svg)](https://github.com/kuoruan/luci-app-kcptun/releases/latest)

## 编译说明

由于目录结构原因，无法使用 OpenWrt/LEDE 的 SDK 直接编译。

需要下载 OpenWrt/LEDE 的完整源码 https://github.com/lede-project/source
并将 luci-app-kcptun 放入 ```feeds/luci/applications/``` 目录下。

```
cd openwrt
git clone https://github.com/kuoruan/luci-app-kcptun.git feeds/luci/applications/luci-app-kcptun
rm -rf tmp/

./scripts/feeds update luci
./scripts/feeds install luci

make menuconfig
make package/luci-app-kcptun/{clean,compile} V=s
```

如果需要在 SDK 上编译，需要自行修改目录结构。参考：https://github.com/shadowsocks/luci-app-shadowsocks

## 安装说明

1. 到 [release](https://github.com/kuoruan/luci-app-kcptun/releases) 页面下载最新版 luci-app-kcptun 和 luci-i18n-kcptun-zh-cn (简体中文翻译文件)
2. 将下载好的 ipk 文件上传到路由器任意目录下, 如 /tmp
3. 先安装 luci-app-kcptun 再安装 luci-i18n-kcptun-zh-cn

```
opkg install luci-app-kcptun_*.ipk
opkg install luci-i18n-kcptun-zh-cn_*.ipk
```

安装好 LuCI 之后，进入配置页面，会提示 ```客户端文件配置有误```。

若路由器上已经有 Kcptun 客户端，直接配置好路径即可，如果当前还没有客户端文件，可以使用以下操作：

1. 配置好 ```客户端文件``` 路径（可选，默认路径 ```/var/kcptun_client```），确保客户端文件所在目录的剩余空间足以放下两个客户端文件；
2. 根据当前设备，配置好 ```CPU 架构```（可选）；
3. 如果你的路由器上装有透明代理软件，最好先打开；
4. 点击 ```检查 Kcptun```，正常情况下应该会获取到 Kcptun 最新版本号；
5. ```点击更新``` 按钮来自动下载配置客户端文件。

如果自动下载更新失败，请手动下载 Kcptun 客户端文件并上传到路由器上，然后配置好 ```客户端文件``` 路径。

下载地址: https://github.com/xtaci/kcptun/releases

注：Golang 的 MIPS/MIPSLE 版本暂时不支持 Soft Float，需要在编译 OpenWrt/LEDE 时打开 ```MIPS FPU Emulator```，其他版本不受影响。

## 卸载说明

卸载时需要先卸载 luci-i18n-kcptun-zh-cn, 再卸载 luci-app-kcptun

```
opkg remove luci-i18n-kcptun-zh-cn
opkg remove luci-app-kcptun
```
