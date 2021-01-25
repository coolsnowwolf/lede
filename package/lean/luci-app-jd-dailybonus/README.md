# luci-app-jd-dailybonus
Luci for JD dailybonus Script for Openwrt  
一个运行在openwrt下的京东签到插件。

### Update Log 2021-01-24  v1.0.2

#### Updates 

- UPD: 大量代码重构，去除lua-request 使用wget实现网络请求节省大量空间和依赖（感谢@Promix953的更新）
- FIX: 修复当使用master版本时 提交信息出错的问题
- FIX: 修复部分文件翻译


详情见[具体日志](./relnotes.txt)。 

### 介绍

使用NobyDa “京东多合一签到脚本”为基础，移植到openwrt实现。  

### 如何使用

假设你的lean openwrt（最新版本19.07） 在 lede 目录下
```
cd lede/package/lean/  

git clone https://github.com/jerrykuku/luci-app-jd-dailybonus.git  

make menuconfig #Check LUCI->Applications->luci-app-jd-dailybonus

make package/lean/luci-app-jd-dailybonus/compile V=s  #单独编译luci-app-jd-dailybonus  

make -j1 V=s #编译固件
```

### 如何安装

🛑 [点击这里去下载最新的版本](https://github.com/jerrykuku/luci-app-jd-dailybonus/releases)

⚠️安装步骤[重要]⚠️  

目前插件已经不再需要node-request 依赖，如果之前有安装的，请到软件包进行卸载。
但是node 是必要依赖，请必须确认已经安装Node。
1.自行安装Node [命令: opkg update && opkg install node]
  (如果已经安装node版本的网易云插件则可以跳过)   
  
2.需要安装以下依赖：
```
opkg update  
opkg install node wget lua
```
如果更新ipk后无法获取二维码  
```
#登录ssh后执行
rm -rf /tmp/luci-indexcache /tmp/luci-modulecache/*
```
3.安装luci-app-jd-dailybonus

### 感谢

感谢[NobyDa](https://github.com/NobyDa) 的脚本。  

### 我的其它项目
Argon theme ：https://github.com/jerrykuku/luci-theme-argon  
Argon theme config  ：https://github.com/jerrykuku/luci-app-argon-config  
Hello World ：https://github.com/jerrykuku/luci-app-vssr  
openwrt-nanopi-r1s-h5 ： https://github.com/jerrykuku/openwrt-nanopi-r1s-h5  

### 支持我
如果你觉得我做的不错，可以赞赏一下。
<img src="https://raw.githubusercontent.com/jerrykuku/staff/master/photo_2019-12-22_11-40-20.jpg" width="300" height="300">