<div align="center">
  <img src="https://raw.githubusercontent.com/jerrykuku/staff/master/Helloworld_title.png"  >
  <h1 align="center">
    An openwrt Internet surfing plug-in
  </h1>
    <h3 align="center">
    HelloWorld是一个以用户最佳主观体验为导向的插件，它支持多种主流协议和多种自定义视频分流服务，拥有精美的操作界面，并配上直观的节点信息。<br><br>
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
    <img src="https://img.shields.io/badge/release-v1.22-blue.svg?">
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


### 更新日志 2021-01-19  v1.22
- FIX: 修复在bootstrap或者其他主题下不能订阅或者其他前端显示出错的Bug。
- FIX: 修复一个导入ss(SIP002) 时带有obfs参数出错的bug。
- FIX: 修复了某些订阅链接中含有相同的节点信息时，重新订阅会增加重复节点的bug。
- UPDATE：更新多个语言翻译。
- UPDATE：将订阅设置从高级设置中拆分到单独的菜单。
- UPDATE：去除分流节点对节点区域的限制。
- UPDATE：增加基于IP的分流规则，请到分流设置中自行添加，IP分流规则使用“IPIfNonMatch” 具体参考[这里](https://www.v2fly.org/config/routing.html#routingobject)【试验功能可能有坑，请谨慎设置】。


详情见[具体日志](./relnotes.txt)。 

### 插件介绍

1. 基于 Lean ssrp 全新MOD的 Hello World ,在原插件的基础上做了一些优化用户操作体验的修改，感谢插件原作者所做出的的努力和贡献！ 
1. 节点列表支持国旗显示并且页面打开自动检测节点的连接时间。  
1. 支持各种分流组合，并且可以自己编辑所有分流的域名，相当于七组自定义分流。  
1. 将节点订阅转移至[高级设置]请悉知。  
1. 底部状态栏：左边显示国旗地区以及IP,右边为四个站点的可访问状态，彩色为可访问，灰色为不能访问。 
1. 优化了国旗匹配方法。  
1. 建议搭配Argon主题，以达到最佳的显示效果。  

欢迎提交bug。

### 如何编译
假设你的Lean openwrt（最新版本19.07） 在 lede 目录下
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
Argon theme ：https://github.com/jerrykuku/luci-theme-argon  
京东签到插件 ：https://github.com/jerrykuku/luci-app-jd-dailybonus  
openwrt-nanopi-r1s-h5 ： https://github.com/jerrykuku/openwrt-nanopi-r1s-h5
