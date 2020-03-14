# luci-theme-argon ([Eng](/README.md))
[1]: https://img.shields.io/badge/license-MIT-brightgreen.svg
[2]: /LICENSE
[3]: https://img.shields.io/badge/PRs-welcome-brightgreen.svg
[4]: https://github.com/jerrykuku/luci-theme-argon/pulls
[5]: https://img.shields.io/badge/Issues-welcome-brightgreen.svg
[6]: https://github.com/jerrykuku/luci-theme-argon/issues/new
[7]: https://img.shields.io/badge/release-v2.1-blue.svg?
[8]: https://github.com/jerrykuku/luci-theme-argon/releases
[9]: https://img.shields.io/github/downloads/jerrykuku/luci-theme-argon/total
[10]: https://img.shields.io/badge/Contact-telegram-blue
[11]: https://t.me/jerryk6
[![license][1]][2]
[![PRs Welcome][3]][4]
[![Issue Welcome][5]][6]
[![Release Version][7]][8]
[![Release Count][9]][8]
[![Contact Me][10]][11]


![](/Screenshots/pc1.jpg)
![](/Screenshots/phone.jpg)
全新的 Openwrt 主题，基于luci-theme-material 和 开源免费的 Argon 模板进行移植。 

## 注意
当前master版本基于官方 OpenWrt 19.07.1 稳定版固件进行移植适配。  
v2.1 适配主线快照版本。  
v2.01 适配官方稳定版 [Luci版本 LuCI openwrt-19.07 branch (git-20.006.26738-35aa527)]  
v1.3 适配18.06 和 Lean Openwrt [如果你是lean代码 请选择这个版本]


## 如何使用
进入 openwrt/package/lean  或者其他目录

####Lean lede
```
cd lede/package/lean  
rm -rf luci-theme-argon  
git clone -b 18.06 https://github.com/jerrykuku/luci-theme-argon.git  
make menuconfig #choose LUCI->Theme->Luci-theme-argon  
make -j1 V=s  
```

####Openwrt SnapShots
```
cd openwrt/package
git clone https://github.com/jerrykuku/luci-theme-argon.git  
make menuconfig #choose LUCI->Theme->Luci-theme-argon  
make -j1 V=s  
```
## 安装
### For Lean openwrt
```
wget --no-check-certificate https://github.com/jerrykuku/luci-theme-argon/releases/download/v1.3/luci-theme-argon_1.4-01-20200101_all.ipk
opkg install luci-theme-argon_1.4-01-20200101_all.ipk
```

### For openwrt 19.07 stable LuCI branch (git-20.006.26738-35aa527)
```
wget --no-check-certificate https://github.com/jerrykuku/luci-theme-argon/releases/download/v2.01/luci-theme-argon_2.01-20200203_all.ipk
opkg install luci-theme-argon_2.01-20200203_all.ipk
```

### For openwrt 19.07 Snapshots LuCI master (git-20.033.77428-3d63732)
```
wget --no-check-certificate https://github.com/jerrykuku/luci-theme-argon/releases/download/v2.1/luci-theme-argon_2.1-20200206_all.ipk
opkg install luci-theme-argon_2.1-20200206_all.ipk
```

## 更新日志 2020.02.06
1. v2.01 修复了当未设置密码时提示错位的问题，同时修正部分下拉框文字颜色问题。  
2. v2.1 适配官方主线快照。

## 截图
![](/Screenshots/pc2.jpg)
![](/Screenshots/pc3.jpg)

## 感谢
luci-theme-material: https://github.com/LuttyYang/luci-theme-material/
