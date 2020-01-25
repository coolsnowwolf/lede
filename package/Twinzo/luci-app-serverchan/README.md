# 说明
- 用于 OpenWRT/LEDE 路由器上进行 Server酱 微信推送的插件
- 基于 serverchan 提供的接口发送信息，Server酱说明：http://sc.ftqq.com/1.version
- 已经自带 luci
- 基于斐讯 k3 制作，不同系统不同设备，请自行修改部分代码
- 依赖 iputils-arping + curl 命令，安装前请 `opkg update`，小内存路由谨慎安装
- 使用主动探测设备连接的方式检测设备在线状态，以避免WiFi休眠机制，主动探测较为耗时，如果有需要请自行调整命令超时设置）
- 流量统计功能依赖 luci-app-wrtbwmon ，自行选装

#### 主要功能
- 路由 ip/ipv6 变动推送
- 设备别名
- 设备上线推送
- 设备离线推送及流量使用情况
- CPU 负载、温度监视
- 定时推送设备运行状态
- MAC 白名单、黑名单、按接口检测设备
- 免打扰

#### 已知问题

- 设备温度命令基于斐讯K3，其他设备如遇到设备温度无法正常读取，请自行修改
`cut -c1-2 /sys/class/thermal/thermal_zone*/temp`
- 潘多拉系统请将脚本开头`#!/bin/sh` 改为 `#!/bin/bash`
- ipaddress 输出的文本有时会有乱码，导致出错，尚不能判定是设备编码环境导致，还是脚本编码导致，先暂时替换部分中文字符
- 直接关闭接口时，该接口的离线设备会忽略检测
- 部分设备无法读取到设备名，脚本依赖 `cat /var/dhcp.leases` 命令读取设备名，如果 dhcp 中不存在设备名，则无法读取设备名

#### ps

- 新功能看情况开发
- 王者荣耀新赛季，继续不思进取中
- 欢迎各种代码提交
- 提交bug时请带上设备信息，和详细的日志与描述
- 三言两句恕我无能为力

![image](https://github.com/tty228/Python-100-Days/blob/master/res/WeChat%E6%88%AA%E5%9C%96_20200111190113.png)
![image](https://github.com/tty228/Python-100-Days/blob/master/res/WeChat%E6%88%AA%E5%9C%96_20200111190912.png)

