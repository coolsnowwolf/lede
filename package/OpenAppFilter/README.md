# OpenAppFilter功能简介

OpenAppFilter模块基于数据流深度识别技术，实现对单个app进行管控的功能，并支持上网记录统计

该模块支持主流linux版本，目前主要在OpenWrt各版本中测试，支持OpenWrt 15.05、OpenWrt 18.06、

OpenWrt 19.07、 LEDE等主流版本

## 主要使用场景
	- 家长对小孩上网行为进行管控，限制小孩玩游戏等
	- 限制员工使用某些app， 如视频、招聘、购物、游戏、常用网站等
	- 记录终端的上网记录，实时了解当前app使用情况，比如xxx正在访问抖音
	
## 插件截图
### 1
![main1](https://github.com/destan19/images/blob/master/oaf/main1.png)


### 2
![main2](https://github.com/destan19/images/blob/master/oaf/main2.png)

## 支持app列表(只列主流)
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

# 使用说明
## 编译说明
- 将该项目clone到openwrt package目录
- make menuconfig 在luci app中选上 luci oaf模块，保存
- 编译生成OpenWrt固件升级， 如果出现页面没有app的情况，保存并应用初始化

## 模块使用限制
- 必须关闭各种加速模块，如软加速、硬加速等
- 模块可能与qos等用到了netfilter mark字段的模块冲突， 自行检查
- 该模块只工作在路由模式， 交换机(桥)模式不会生效
- 可能会存在小概率误判的情况，特别是同一个公司的app，比如淘宝、支付宝等，如果需要过滤，建议相似的app全部勾选
- 抖音等视频软件，会出现缓存，多刷几次再测试是否能过滤

# 技术支持

- 微信公众号: OpenWrt

- 技术交流QQ群: 943396288
