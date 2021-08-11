# 改名公告
#### 2021年04月25日 起luci-app-serverchand 改名为 luci-app-pushbot

如需拉取编译
请把：

`# git clone https://github.com/zzsj0928/luci-app-serverchand package/luci-app-serverchand`

改为

`git clone https://github.com/zzsj0928/luci-app-pushbot package/luci-app-pushbot`

并把 .config 中

`CONFIG_PACKAGE_luci-app-serverchand=y`

改为

`CONFIG_PACKAGE_luci-app-pushbot=y`

注意：本次改名需要提前备份serverchand配置，并于PushBot中重新配置。

再次谢谢各位支持

# 申明
- 本插件由[tty228/luci-app-serverchan](https://github.com/tty228/luci-app-serverchan)原创.
- 因微信推送存在诸多弊端（无法分开聊天工具与功能性消息推送，通知内不显示内容，内容需要点开才能查看等）,
- 故由  然后七年  @zzsj0928 重新修改为本插件，为钉钉机器人API使用。
- 本插件工作在：openwrt
- 本插件支持：钉钉推送,企业微信推送,PushPlus推送,微信推送,企业微信应用推送,飞书推送,钉钉机器人推送,企业微信机器人推送,飞书机器人推送,一对多推送,Bark推送(仅iOS)
# 显示效果
## 通知栏：直接显示推送主题，一目了然，按设备不同，分组显示
<img src="https://raw.githubusercontent.com/zzsj0928/ReadmeContents/main/Serverchand/Msg.Notification.jpg" width="500">

## 消息列表：直接显示最新推送的标题
<img src="https://raw.githubusercontent.com/zzsj0928/ReadmeContents/main/Serverchand/Msg.List.jpg" width="500">

## 消息内容：直接显示所有推送信息，不用二次点开再查看
<img src="https://raw.githubusercontent.com/zzsj0928/ReadmeContents/main/Serverchand/MsgContentDetials.jpeg" width="500">

# 下载
- [luci-app-pushbot](https://github.com/zzsj0928/luci-app-pushbot/releases)


-----------------------------------------------------
#####################################################
-----------------------------------------------------

# 以下为原插件简介：

# 简介
- 用于 OpenWRT/LEDE 路由器上进行 Server酱 微信/Telegram 推送的插件
- 基于 serverchan 提供的接口发送信息，Server酱说明：http://sc.ftqq.com/1.version
- **基于斐讯 k3 制作，不同系统不同设备，请自行修改部分代码，无测试条件无法重现的 bug 不考虑修复**
- 依赖 iputils-arping + curl 命令，安装前请 `opkg update`，小内存路由谨慎安装
- 使用主动探测设备连接的方式检测设备在线状态，以避免WiFi休眠机制，主动探测较为耗时，**如遇设备休眠频繁，请自行调整超时设置**
- 流量统计功能依赖 wrtbwmon ，自行选装或编译，该插件与 Routing/NAT 、Flow Offloading 冲突，开启无法获取流量，自行选择，L大版本直接编译 luci-app-wrtbwmon

#### 主要功能
- 路由 ip/ipv6 变动推送
- 设备别名
- 设备上线推送
- 设备离线推送及流量使用情况
- CPU 负载、温度监视
- 定时推送设备运行状态
- MAC 白名单、黑名单、按接口检测设备
- 免打扰
- 无人值守任务

#### 说明
- 潘多拉系统、或不支持 sh 的系统，请将脚本开头 `#!/bin/sh` 改为 `#!/bin/bash`，或手动安装 `sh`
- 追新是没有意义的，没有问题没必要更新，上班事情忙完了，摸鱼又不会摸，只能靠写几行 bug ，才能缓解无聊这样子

#### 已知问题
- 直接关闭接口时，该接口的离线设备会忽略检测
- 部分设备无法读取到设备名，脚本使用 `cat /var/dhcp.leases` 命令读取设备名，如果 dhcp 中不存在设备名，则无法读取设备名（如二级路由设备、静态ip设备），请使用设备名备注

# Download
- [luci-app-serverchan](https://github.com/tty228/luci-app-serverchan/releases)
- [wrtbwmon](https://github.com/brvphoenix/wrtbwmon)
- [luci-app-wrtbwmon](https://github.com/brvphoenix/luci-app-wrtbwmon) 

#### ps
- 新功能看情况开发
- 王者荣耀新赛季，不思进取中
- 欢迎各种代码提交
- 提交bug时请尽量带上设备信息，日志与描述（如执行`/usr/bin/serverchan/serverchan`后的提示、日志信息、/tmp/serverchan/ipAddress 文件信息）
- 三言两句恕我无能为力
- 武汉加油

