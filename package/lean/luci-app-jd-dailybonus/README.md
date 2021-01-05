# luci-app-jd-dailybonus
Luci for JD dailybonus Script for Openwrt  
一个运行在openwrt下的京东签到插件。

### Update Log 2020-12-30  

#### Updates

- UPD: 取消已经无法使用的crx工具，请使用压缩包手动安装扩展。
- UPD: 修改了获取京东cookie的插件，只获取必要的cookie信息，信息更加简短。
- UPD: 后台更新取消了版本大小检测，每次更新将会直接拉取源脚本。
- UPD: 前端手动更新脚本，点击后如果确认有新版本将自动更新到新版本。


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
  
2.安装luci-app-jd-dailybonus

### 感谢

感谢[NobyDa](https://github.com/NobyDa) 的脚本。  

### 我的其它项目
Argon theme ：https://github.com/jerrykuku/luci-theme-argon  
Argon theme config  ：https://github.com/jerrykuku/luci-app-argon-config
Hello World ：https://github.com/jerrykuku/luci-app-vssr  
openwrt-nanopi-r1s-h5 ： https://github.com/jerrykuku/openwrt-nanopi-r1s-h5  
