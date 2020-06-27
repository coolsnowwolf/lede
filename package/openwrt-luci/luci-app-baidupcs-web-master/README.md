# luci-app-baidupcs-web-master
本插件为[BaiduPCS-Web](https://github.com/liuzhuoling2011/baidupcs-web)的LuCI配置页面, 用于启动以及配置BaiduPCS-Web. 

部分代码来自[[CTCGFW]](https://github.com/project-openwrt/BaiduPCS-Web/tree/master/luci-app-baidupcs-web).

此插件依赖baidupcs-web软件包, 你可以在[此处](https://github.com/KFERMercer/openwrt-baidupcs-web)找到.

## 如何添加插件 

首先, cd 进你本地的 OpenWrt 项目根目录下, 然后依次执行以下命令: 

`git clone https://github.com/KFERMercer/openwrt-baidupcs-web.git ./package/baidupcs-web`

`git clone https://github.com/KFERMercer/luci-app-baidupcs-web.git ./package/luci-app-baidupcs-web`

`rm -rf ./tmp/`

`make menuconfig`

在编译配置菜单中选择`luci-app-baidupcs-web`. 
