# luci-theme-argon ([中文](/README_ZH.md))

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

A new Luci theme for LEDE/OpenWRT  
Argon is a clean HTML5 theme for LuCI. It is based on luci-theme-material and Argon Template  


The old version is still in another branch call old. If you need that you can checkout that branch.

## Notice
v2.01 Adapt to official 19.07 stable version (not snapshot) LuCI openwrt-19.07 branch (git-20.006.26738-35aa527).  
v2.1 Adapt to official mainline snapshot.  
You can checkout branch 18.06 for  OpenWRT 18.06 or lean 19.07.

## How to build

Enter in your openwrt/package/lean  or  other
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

## Install 
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

## Update log 2020.02.06
1. v2.01  Fix display error, when not set password.
2. v2.1 Adapt to official mainline snapshot.

## More Screenshots

![](/Screenshots/pc2.jpg)
![](/Screenshots/pc3.jpg)

## Thanks to 
luci-theme-material: https://github.com/LuttyYang/luci-theme-material/
