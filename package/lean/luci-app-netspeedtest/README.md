[![若部分图片无法正常显示，请挂上机场浏览或点这里到末尾看修复教程](https://visitor-badge.glitch.me/badge?page_id=sirpdboy-visitor-badge)](#解决-github-网页上图片显示失败的问题) [![](https://img.shields.io/badge/TG群-点击加入-FFFFFF.svg)](https://t.me/joinchat/AAAAAEpRF88NfOK5vBXGBQ)
<a href="#readme">
    <img src="https://img.vim-cn.com/a1/8713845a4aa922ac96619b0d2fb3d6919d37fc.png" alt="图飞了😂" title="NetSpeedTest" align="right" height="180" />
</a>

欢迎来到sirpdboy的源码仓库！
=
Welcome to sirpdboy's  git source of packages
-
[luci-app-NetSpeedTest — 网络速度测试1.5完整版](https://github.com/sirpdboy/NetSpeedTest)
======================

[![](https://img.shields.io/badge/-目录:-696969.svg)](#readme) [![](https://img.shields.io/badge/-写在前面-F5F5F5.svg)](#写在前面-) [![](https://img.shields.io/badge/-编译说明-F5F5F5.svg)](#编译说明-) [![](https://img.shields.io/badge/-说明-F5F5F5.svg)](#说明-) [![](https://img.shields.io/badge/-捐助-F5F5F5.svg)](#捐助-) 

请 **认真阅读完毕** 本页面，本页面包含注意事项和如何使用。

a new NetSpeedTest luci app bese luci-app-NetSpeedTest
-

## 写在前面：[![](https://img.shields.io/badge/-写在前面-F5F5F5.svg)](#写在前面-)

 一直在找OPENWRT上测试速度的插件，苦寻不到，于是有了它!
此插件可进行内外和外网网络速度测试。

1.内网测速基于iperf3 插件，服务端路由器如果没有安装请先安装此ipk插件。

2.测速的终端使用机器必须和测速服务器在同一个局域网络中！

3.客户端使用步骤：
  启动测速服务器端-->下载测试客户端-->运行测速客户端-->输入服务端IP地址-->查看结果。

5.客户端运行，国内端下载中有“iperf3测速客户端”，运行它输入服务器IP即可。
  国外原版，需要手动进入 CMD命令模式，再输入命令：iperf3.exe -c 服务器IP 

6.网络测速iperf3客户端下载地址：https://sipdboy.lanzoui.com/b01c3esih 密码:cpd6

8.外网测速使用speedtest.net测速内核，需要依赖speedtest,另外感谢superspeed和user1121114685因为借用了你们的灵感！

9.外网测速最后测试阶段感谢佐大：佐须之男 测试查问题！

10.新插件难免有bug 请不要大惊小怪。欢迎提交bug。

## 编译说明 [![](https://img.shields.io/badge/-编译说明-F5F5F5.svg)](#编译说明-) 

将NetSpeedTest 主题添加至 LEDE/OpenWRT 源码的方法。

## 下载源码方法一：
编辑源码文件夹根目录feeds.conf.default并加入如下内容:

```Brach
    # feeds获取源码：
    src-git NetSpeedTest  https://github.com/sirpdboy/NetSpeedTest
 ``` 
  ```Brach
   # 更新feeds，并安装主题：
    scripts/feeds update NetSpeedTest
	scripts/feeds install luci-app-NetSpeedTest
 ``` 	

## 下载源码方法二：
 ```Brach
    # 下载源码
    
    git clone https://github.com/sirpdboy/NetSpeedTest package/NetSpeedTest
    
    make menuconfig
 ``` 
## 配置菜单
 ```Brach
    make menuconfig
	# 找到 LuCI -> Applications, 选择 luci-app-NetSpeedTest, 保存后退出。
 ``` 
## 编译
 ```Brach 
    # 编译固件
    make package/NetSpeedTest/luci-app-NetSpeedTest/{clean,compile} V=s
```   

## 说明 [![](https://img.shields.io/badge/-说明-F5F5F5.svg)](#说明-)

源码来源：https://github.com/sirpdboy/NetSpeedTest/luci-app-NetSpeedTest


你可以随意使用其中的源码，但请注明出处。
============================

# My other project
网络速度测试 ：https://github.com/sirpdboy/NetSpeedTest

定时关机重启 : https://github.com/sirpdboy/luci-app-autopoweroff

关机功能插件 : https://github.com/sirpdboy/luci-app-poweroffdevice

opentopd主题 : https://github.com/sirpdboy/luci-theme-opentopd

opentoks 主题: https://github.com/sirpdboy/luci-theme-opentoks [仿KOOLSAHRE主题]

btmob 主题: https://github.com/sirpdboy/luci-theme-btmob

系统高级设置 : https://github.com/sirpdboy/luci-app-advanced

## 捐助 [![](https://img.shields.io/badge/-捐助-F5F5F5.svg)](#捐助-) 

**如果你觉得此项目对你有帮助，请捐助我们，以使项目能持续发展，更加完善。··请作者喝杯咖啡~~~**

**你们的支持就是我的动力！**

### 捐助方式

|     <img src="https://img.shields.io/badge/-支付宝-F5F5F5.svg" href="#赞助支持本项目-" height="25" alt="图飞了😂"/>  |  <img src="https://img.shields.io/badge/-微信-F5F5F5.svg" height="25" alt="图飞了😂" href="#赞助支持本项目-"/>  | 
| :-----------------: | :-------------: |
|<img src="https://img.vim-cn.com/fd/8e2793362ac3510094961b04407beec569b2b4.png" width="150" height="150" alt="图飞了😂" href="#赞助支持本项目-"/>|<img src="https://img.vim-cn.com/c7/675730a88accebf37a97d9e84e33529322b6e9.png" width="150" height="150" alt="图飞了😂" href="#赞助支持本项目-"/>|

<a href="#readme">
    <img src="https://img.shields.io/badge/-返回顶部-orange.svg" alt="图飞了😂" title="返回顶部" align="right"/>
</a>

###### [解决 Github 网页上图片显示失败的问题](https://blog.csdn.net/qq_38232598/article/details/91346392)

[![](https://img.shields.io/badge/TG群-点击加入-FFFFFF.svg)](https://t.me/joinchat/AAAAAEpRF88NfOK5vBXGBQ)

