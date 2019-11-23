# 说明
- 用于 OpenWRT/LEDE 路由器上进行 Server酱 微信推送的插件
- 基于 serverchan 提供的接口发送信息，Server酱说明：http://sc.ftqq.com/1.version
- 已经自带 luci

#### 主要功能
- 路由 ip 变动推送
- 设备别名
- 设备上下线推送
- CPU 负载、温度监视
- 定时推送设备运行状态
- MAC 白名单、黑名单、仅检测某接口设备
- 免打扰时间

#### 已知BUG

- 多拨环境下无法获取 wan ip，详情查看https://github.com/tty228/luci-app-serverchan/issues/8
- 可能因为编译环境不同或者别的原因，makefile 提升权限失败，造成无法启动，对makefile没研究过，不太了解
如遇到此问题请查阅 @zxlhhyccc 的解决方案
https://github.com/tty228/luci-app-serverchan/issues/1
- 由于不同设备温度文件不一样，如遇到设备温度无法正常读取，请修改
“cut -c1-2 /sys/class/thermal/thermal_zone0/temp” @KFERMercer 

#### 咳

- 前段时间比较忙，各种事情，所以失踪了几个月
- 接下来的时间……
- 大概沉迷怀旧服
- 尴尬又不失礼貌的微笑（划掉）
