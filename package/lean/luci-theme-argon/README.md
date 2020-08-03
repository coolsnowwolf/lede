# luci-theme-argon ([中文](/README_ZH.md))

[1]: https://img.shields.io/badge/license-MIT-brightgreen.svg
[2]: /LICENSE
[3]: https://img.shields.io/badge/PRs-welcome-brightgreen.svg
[4]: https://github.com/jerrykuku/luci-theme-argon/pulls
[5]: https://img.shields.io/badge/Issues-welcome-brightgreen.svg
[6]: https://github.com/jerrykuku/luci-theme-argon/issues/new
[7]: https://img.shields.io/badge/release-v1.6.3-blue.svg?
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


A new Luci theme for LEDE/OpenWRT  
Argon is a clean HTML5 theme for LuCI. It is based on luci-theme-material and Argon Template  

## Notice

This branch only matches lean openwrt LuCI 18.06.

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

## Install 

### For Lean openwrt 18.06 LuCI
```
wget --no-check-certificate https://github.com/jerrykuku/luci-theme-argon/releases/download/v1.6.4/luci-theme-argon_1.6.4-20200727_all.ipk
opkg install luci-theme-argon*.ipk
```

## Update log 2020.07.27 [18.06] V1.6.5 

- New: login background image can be customized now, upload image to /www/luci-static/argon/background/ (only jpg, png and gif are allowed). Uploaded images will be displayed if they are present. If you have multiple images in /www/luci-static/argon/background/, they will be displayed randomly upon each login.【v1.6.4】
- New: force dark mode. Dark mode can now be enabled without client being in "dark mode". To enable: ssh into your router and enter "touch /etc/dark", to disable enter "rm -rf touch /etc/dark" (automatic dark mode).【v1.6.4】
- New: Argon Version displayed in footer will match ipk version from now on.【v1.6.4】
- Fix: Font colors. 【v1.6.4】

- Add blur effect for login form 【v1.6.3】

- New login theme, Request background imge from bing.com, Auto change everyday. 【v1.6.1】
- New theme icon 【v1.6.1】
- Add more menu category  icon 【v1.6.1】
- Fix font-size and padding margin 【v1.6.1】
- Restructure css file 【v1.6.1】
- Auto adapt to dark mode 【v1.6.1】


## Screenshots 
![](/Screenshots/pc/screenshot1.jpg)
![](/Screenshots/pc/screenshot2.jpg)
![](/Screenshots/pc/screenshot3.jpg)
![](/Screenshots/phone/Screenshot_1.jpg)
![](/Screenshots/phone/Screenshot_2.jpg)

## Thanks to 
luci-theme-material: https://github.com/LuttyYang/luci-theme-material/
