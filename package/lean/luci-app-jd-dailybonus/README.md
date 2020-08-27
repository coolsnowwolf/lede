# luci-app-jd-dailybonus
Luci for JD dailybonus Script for Openwrt  
一个运行在openwrt下的京东签到插件。

### Update Log 2020-08-26  

#### Updates

- FIX: 增加一个可选的更新源，如果github源无法更新，请选择gitee。


详情见[具体日志](./relnotes.txt)。 

### 介绍

使用NobyDa “京东多合一签到脚本”为基础，移植到openwrt实现。  

### 如何使用

假设你的lean openwrt（最新版本19.07） 在 lede 目录下
```
cd lede/package/lean/  

git clone https://github.com/jerrykuku/node-request.git  #git node-request 依赖

git clone https://github.com/jerrykuku/luci-app-jd-dailybonus.git  

make menuconfig #Check LUCI->Applications->luci-app-jd-dailybonus

make package/lean/node-request/compile V=s  #单独编译node-request 依赖  

make package/lean/luci-app-jd-dailybonus/compile V=s  #单独编译luci-app-jd-dailybonus  

make -j1 V=s #编译固件
```

### 如何安装

🛑 [点击这里去下载最新的版本](https://github.com/jerrykuku/luci-app-jd-dailybonus/releases)

⚠️安装步骤[重要]⚠️  

1.自行安装Node [命令: opkg update && opkg install node]
  (如果已经安装node版本的网易云插件则可以跳过)   
  
2.一些必要的依赖 wget 和 coreutils-nohup [命令：opkg install wget coreutils-nohup]
  
3.根据自己的系统架构选择合适的node-request.ipk进行安装。  
  [node-request下载](https://github.com/jerrykuku/luci-app-jd-dailybonus/releases/tag/0.7.6) 
  目前的node-request 已经不再区分系统架构 请直接安装 node-request_2.88.2-1-all.ipk

4.安装luci-app-jd-dailybonus

### 感谢

感谢[NobyDa](https://github.com/NobyDa) 的脚本。  

### 我的其它项目
Argon theme ：https://github.com/jerrykuku/luci-theme-argon  
Hello World ：https://github.com/jerrykuku/luci-app-vssr  
openwrt-nanopi-r1s-h5 ： https://github.com/jerrykuku/openwrt-nanopi-r1s-h5  
