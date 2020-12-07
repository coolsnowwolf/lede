
<div align="center">
  <img src="https://raw.githubusercontent.com/jerrykuku/staff/master/Helloworld_title.png"  >
  <h1 align="center">
    An openwrt Internet surfing plug-in
  </h1>
    <h3 align="center">
    HelloWorld是一个以用户最佳主观体验为导向的插件，它支持多种主流协议和多种自定义视频分流服务，拥有精美的操作界面，并配上直观的节点信息。
  </h3>

  <a href="/LICENSE">
    <img src="https://img.shields.io/badge/license-MIT-brightgreen.svg" alt="">
  </a>

  <a href="https://github.com/jerrykuku/luci-app-vssr/pulls">
    <img src="https://img.shields.io/badge/PRs-welcome-brightgreen.svg" alt="">
  </a>
  
  <a href="https://github.com/jerrykuku/luci-app-vssr/issues/new">
    <img src="https://img.shields.io/badge/Issues-welcome-brightgreen.svg">
  </a>
  
  <a href="https://github.com/jerrykuku/luci-app-vssr/releases">
    <img src="https://img.shields.io/badge/release-v1.20-blue.svg?">
  </a>
  
  <a href="https://github.com/jerrykuku/luci-app-vssr/releases">
    <img src="https://img.shields.io/github/downloads/jerrykuku/luci-app-vssr/total">
  </a>
  
  <a href="https://t.me/PIN1Group">
    <img src="https://img.shields.io/badge/Contact-telegram-blue">
  </a>
</div>


<b><br>支持全部类型的节点分流</b>  
目前只适配最新版 argon主题 （其他主题下应该也可以用 但显示应该不会很完美）  
目前Lean最新版本的openwrt 已经可以直接拉取源码到 package/lean 下直接进行勾选并编译。  


### 更新日志 2020-12-06  v1.20
- NEW：局域网访问控制增加列表内或列表外代理的选项。
- NEW：现在所有的分流域名都可以自定义了，具体参见新的功能菜单 分流设置。
- FIX：修复了在导入trojan链接时，不能导入密码的错误。
- FIX：修复一些应用程序路径检测错误。

详情见[具体日志](./relnotes.txt)。 

### 插件介绍

1. 基于lean ssr+ 全新修改的Vssr（更名为Hello World）主要做了很多的修改，同时感谢插件原作者所做出的的努力和贡献！ 
1. 节点列表支持国旗显示并且页面打开自动ping。  
1. 支持各种分流组合，并且可以自己编辑所有分流的域名，相当于七组自定义分流。  
1. 将节点订阅转移至[高级设置]请悉知。  
1. 底部状态栏：左边显示国旗地区以及IP,右边为四个站点的可访问状态，彩色为可访问，灰色为不能访问。 
1. 优化了国旗匹配方法，在部分带有emoji counrty code的节点名称中优先使用 emoji code 匹配国旗。  
1. 建议搭配argon theme，能有最好的显示体验。  

新修改插件难免有bug 请不要大惊小怪。欢迎提交bug。

### 如何编译
假设你的lean openwrt（最新版本19.07） 在 lede 目录下
```
cd lede/package/lean/  

git clone https://github.com/jerrykuku/lua-maxminddb.git  #git lua-maxminddb 依赖

git clone https://github.com/jerrykuku/luci-app-vssr.git  

make menuconfig

make -j1 V=s
```

### 感谢
https://github.com/coolsnowwolf/lede

### 我的其它项目
Argon theme ：https://github.com/jerrykuku/luci-app-vssr  
京东签到插件 ：https://github.com/jerrykuku/luci-app-jd-dailybonus  
openwrt-nanopi-r1s-h5 ： https://github.com/jerrykuku/openwrt-nanopi-r1s-h5
