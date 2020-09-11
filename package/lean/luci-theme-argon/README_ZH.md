# luci-theme-argon ([English](/README.md))
[1]: https://img.shields.io/badge/license-MIT-brightgreen.svg
[2]: /LICENSE
[3]: https://img.shields.io/badge/PRs-welcome-brightgreen.svg
[4]: https://github.com/jerrykuku/luci-theme-argon/pulls
[5]: https://img.shields.io/badge/Issues-welcome-brightgreen.svg
[6]: https://github.com/jerrykuku/luci-theme-argon/issues/new
[7]: https://img.shields.io/badge/release-v2.2.4-blue.svg?
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

![](/Screenshots/screenshot_pc.jpg)
![](/Screenshots/screenshot_phone.jpg)

全新的 Openwrt 主题，基于luci-theme-material 和 开源免费的 Argon 模板进行移植。 

## 注意

当前master版本基于官方 OpenWrt 19.07.1  稳定版固件进行移植适配。  
v2.x.x 适配主线快照版本。  
v1.x.x 适配18.06 和 Lean Openwrt [如果你是lean代码 请选择这个版本]


## 更新日志 2020.08.21 v2.2.4

- 【v2.2.4】修复了在某些手机下图片背景第一次加载不能显示的问题。
- 【v2.2.4】取消 luasocket 的依赖，无需再担心依赖问题。
- 【v2.2.3】修正了在暗色模式下，固件刷写弹窗内的显示错误。
- 【v2.2.3】更新了图标库，为未定义的菜单增加了一个默认的图标。
- 【v2.2.2】背景文件策略调整为，同时接受 jpg png gif mp4, 自行上传文件至 /www/luci-static/argon/background 图片和视频同时随机。
- 【v2.2.2】增加强制暗色模式，进入ssh 输入 "touch /etc/dark" 进行开启。
- 【v2.2.2】视频背景加了一个音量开关，喜欢带声音的可以自行点击开启，默认为静音模式。
- 【v2.2.2】修复了手机模式下，登录页面出现键盘时，文字覆盖按钮的问题。
- 【v2.2.2】修正了暗黑模式下下拉选项的背景颜色，同时修改了滚动条的样式。
- 【v2.2.2】jquery 更新到 v3.5.1。
- 【v2.2.2】获取Bing Api 的方法从wget 更新到luasocket 并添加依赖。
- 【v2.2.1】登录背景添加毛玻璃效果。
- 【v2.2.1】全新的登录界面,图片背景跟随Bing.com，每天自动切换。
- 【v2.2.1】全新的主题icon。
- 【v2.2.1】增加多个导航icon。
- 【v2.2.1】细致的微调了 字号大小边距等等。
- 【v2.2.1】重构了css文件。
- 【v2.2.1】自动适应的暗黑模式。

## 如何编译

进入 openwrt/package/lean  或者其他目录

### Lean源码

```
cd lede/package/lean  
rm -rf luci-theme-argon  
git clone -b 18.06 https://github.com/jerrykuku/luci-theme-argon.git  
make menuconfig #choose LUCI->Theme->Luci-theme-argon  
make -j1 V=s  
```

### Openwrt 官方源码

```
cd openwrt/package
git clone https://github.com/jerrykuku/luci-theme-argon.git  
make menuconfig #choose LUCI->Theme->Luci-theme-argon  
make -j1 V=s  
```

## 如何安装

### Lean源码

```
wget --no-check-certificate https://github.com/jerrykuku/luci-theme-argon/releases/download/v1.6.9/luci-theme-argon_1.6.9-20200821_all.ipk
opkg install luci-theme-argon*.ipk
```

### For openwrt official 19.07 Snapshots LuCI master 

```
wget --no-check-certificate https://github.com/jerrykuku/luci-theme-argon/releases/download/v2.2.4/luci-theme-argon_2.2.4-20200821_all.ipk
opkg install luci-theme-argon*.ipk
```

## 感谢

luci-theme-material: https://github.com/LuttyYang/luci-theme-material/
