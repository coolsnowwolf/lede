# luci-app-jd-dailybonus
Luci for JD dailybonus Script for Openwrt  
一个运行在openwrt下的京东签到插件。

### Update Log 2021-01-20  

#### Updates 

- UPD: 大量代码重构，版本更新到1.0.0
- UPD: 采用扫码获取cookie(基于Lua 脚本实现)
- UPD: 增加超时时间设置
- UPD: 取消了当失败时推送消息的设定每次都会推送 包括手动
- UPD: 增加telegram推送
- UPD: 无需修改js脚本设置cookie，才用本地json缓存来读取配置文件
- UPD: 签到时间拆分为小时+分钟
- UPD: 取消语言文件，默认为中文
- UPD: 兼容性做了适配，兼容boorstrap
- UPD: 默认JS脚本更新到v1.90


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
opkg install luasocket lua-md5 lua-cjson luasec
```
3.安装luci-app-jd-dailybonus

### 感谢

感谢[NobyDa](https://github.com/NobyDa) 的脚本。  

### 我的其它项目
Argon theme ：https://github.com/jerrykuku/luci-theme-argon  
Argon theme config  ：https://github.com/jerrykuku/luci-app-argon-config
Hello World ：https://github.com/jerrykuku/luci-app-vssr  
openwrt-nanopi-r1s-h5 ： https://github.com/jerrykuku/openwrt-nanopi-r1s-h5  
