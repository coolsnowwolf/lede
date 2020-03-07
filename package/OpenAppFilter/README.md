## OpenAppFilter功能简介

OpenAppFilter模块基于数据流深度识别技术，实现对单个app进行管控的功能，并支持上网记录统计

该模块支持主流linux版本，目前主要在OpenWrt各版本中测试，支持OpenWrt 15.05、OpenWrt 18.06、

OpenWrt 19.07、 LEDE等主流版本

### 主要使用场景
	- 家长对小孩上网行为进行管控，限制小孩玩游戏等
	- 限制员工使用某些app， 如视频、招聘、购物、游戏、常用网站等
	- 记录终端的上网记录，实时了解当前app使用情况，比如xxx正在访问抖音
	
### 插件截图
#### 1
![main1](https://github.com/destan19/images/blob/master/oaf/main1.png)


#### 2
![main2](https://github.com/destan19/images/blob/master/oaf/main2.png)

### 支持app列表(只列主流)
 - 游戏
   王者荣耀 英雄联盟 欢乐斗地主 梦幻西游 明日之后 ...
 - 音乐
 - 购物
   淘宝 京东 唯品会 拼多多 苏宁易购
 - 聊天
	QQ 微信 钉钉 
 - 招聘
 - 视频
   抖音小视频 斗鱼直播 腾讯视频 爱奇艺 火山小视频 YY 微视 虎牙直播 快手 小红书 ...

## 编译说明
1. 下载OpenWrt源码，并完成编译(OpenWrt公众号有相关教程）
> git clone https://github.com/destan19/openwrt.git
2. 下载应用过滤源码放到OpenWrt的package 目录
> cd package  
git clone https://github.com/destan19/OpenAppFilter.git  
包含三个文件夹  
luci-app-oaf   		luci界面安装包，包含中文语言包  
oaf   			内核模块包  
open-app-filter  	应用层服务和脚本  
	
3. make menuconfig, 在luci app中选上luci oaf app模块并保存

4. 编译应用过滤模块
- 方案1： OpenWrt项目全部重新编译   make V=s 
- 方案2:  只编译应用过滤单个模块
> make package/oaf/compile V=s  
 make package/open-app-filter/compile V=s  
 make package/luci-app-oaf/compile V=s  
 编译完成后在bin目录的子目录中会出现四个ipk文件  
 可以直接用  
 find bin/ -name "*oaf*"  
 find bin/ -name "*appfilter*"   
 查询出ipk文件的位置  

## 使用说明
- 建议小白直接刷固件来只用应用过滤功能，因为ipk文件安装很可能安装失败，因为版本不一致。
- 刷机后第一次可能出现页面没有显示app列表的问题，可以保存下进行初始化。
- 建议用18.06及以上版本，低版本可能存在访问记录luci页面问题。
## 使用限制
- 必须关闭各种加速模块，如软加速、硬加速等
- 模块可能与qos等用到了netfilter mark字段的模块冲突， 自行检查
- 该模块只工作在路由模式， 交换机(桥)模式不会生效
- 可能会存在小概率误判的情况，特别是同一个公司的app，比如淘宝、支付宝等，如果需要过滤，建议相似的app全部勾选
- 抖音等视频软件，会出现缓存，多刷几次再测试是否能过滤
- 可能出现某些app不能过滤的问题，一个可能是app特征码改变，也可能是添加的特征库存在问题，可以在Issues中反馈，后面尽可能快速更新

## 技术支持

- 微信公众号: OpenWrt (获取应用过滤最新固件和OpenWrt开发教程)

- 技术交流QQ群（1000人）: 943396288
