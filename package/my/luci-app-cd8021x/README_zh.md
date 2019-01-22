# luci-app-cd8021x

[README](README.md) | [中文说明](README_zh.md)

OpenWrt 802.1x 有线认证拨号界面<br/>

![screenshot](https://raw.githubusercontent.com/max0y/luci-app-cd8021x/master/screenshot_zh.png)<br/>
## 安装<br/>
1. 这个包的功能实现依赖于*wpad*，需要先卸载*wpad-mini*，再安装*wpad*，也可以手动到OpenWrt[官方源](https://downloads.openwrt.org/releases/17.01.4/packages/)处下载*wpad*安装包
```bash
opkg update
opkg remove wpad-mini
opkg install wpad
```
2. 然后在[release][release_url]页面下载最新版luci-app-cd8021x
3. 将下载的ipk文件上传至路由器/tmp目录
4. 安装luci-app-cd8021x
```bash
cd /tmp
opkg install luci-app-cd8021x_*.ipk
```
## 编译<br/>
如果需要自己编译安装包，请使用 OpenWrt 的 [SDK][openwrt_sdk_url] 编译， SDK使用注意事项：[Using the SDK][openwrt_sdk_usage_url]
```bash
# 解压下载好的 SDK
tar -xvf openwrt-sdk-ar71xx-generic_gcc-*.tar.xz
cd openwrt-sdk-ar71xx-*

# Clone 项目
git clone https://github.com/max0y/luci-app-cd8021x.git package/luci-app-cd8021x

# 编译 po2lmo (如果已安装po2lmo可跳过)
pushd package/luci-app-cd8021x/tool/po2lmo
make && sudo make install
popd

# 运行make menuconfig，选择要编译的包 LuCI -> 3. Applications
make menuconfig

# 开始编译
make package/luci-app-cd8021x/compile V=99
```

[release_url]: https://github.com/max0y/luci-app-cd8021x/releases
[openwrt_sdk_url]: https://downloads.lede-project.org/snapshots/targets/ar71xx/generic
[openwrt_sdk_usage_url]: https://openwrt.org/docs/guide-developer/using_the_sdk
