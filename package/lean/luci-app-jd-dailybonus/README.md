# luci-app-jd-dailybonus
Luci for JD dailybonus Script for Openwrt  
一个运行在openwrt下的京东签到插件。

### Update Log 2020-07-10  

#### Updates

- UPDATE: 对整个插件进行了重构，cookie更新机制做了修改。
- UPDATE: 增加新的栏目，可以自行编辑脚本的cookie。
- UPDATE: 增加每天定时开关，可从脚本作者源更新脚本。
- UPDATE: 将当前的打开插件自动检测版本，修改为手动检查，检查到新版本可点击更新。

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

[点击这里去下载最新的版本](https://github.com/jerrykuku/luci-app-jd-dailybonus/releases)

### 感谢

感谢[NobyDa](https://github.com/NobyDa) 的脚本。  

### 我的其它项目
Argon theme ：https://github.com/jerrykuku/luci-theme-argon  
Hello World ：https://github.com/jerrykuku/luci-app-vssr  
openwrt-nanopi-r1s-h5 ： https://github.com/jerrykuku/openwrt-nanopi-r1s-h5  
