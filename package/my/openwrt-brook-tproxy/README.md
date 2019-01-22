OpenWrt Luci for Brook Transparent Proxy
===


简介
---

 本软件包是 Brook Tproxy 的 LuCI 控制界面, 方便用户控制和使用「透明代理」功能
 

特性
---

软件包包含 [brook][1] 的可执行文件，以及luci控制界面  

支持两种运行模式：IP路由模式和GFW列表模式（支持自定义GFWList）

运行模式介绍
---
【IP路由模式】
 - 所有国内IP网段不走代理，国外IP网段走代理
 - 白名单模式：缺省都走代理，列表中IP网段不走代理

【GFW列表模式】
 - 只有在GFW列表中的网站走代理；其他都不走代理
 - 黑名单模式：缺省都不走代理，列表中网站走代理

编译
---

 - 从 OpenWrt 的 [SDK][2] 编译
 
   ```bash
   # 以 ar71xx 平台为例
   tar xjf OpenWrt-SDK-15.05-ar71xx-generic_gcc-4.8-linaro_uClibc-0.9.33.2.Linux-x86_64.tar.bz2
   cd OpenWrt-SDK-*

   # Clone 项目
   git clone https://github.com/WouldChar/openwrt-brook-tproxy.git package/openwrt-brook-tproxy

   # 如果没有安装po2lmo，则安装（可选）
   pushd package/openwrt-brook-tproxy/tools/po2lmo
   make && sudo make install
   popd

   # 选择要编译的包 
   #luci ->3. Applications ->openwrt-brook-tproxy
   make menuconfig

   # 开始编译
   make package/openwrt-brook-tproxy/compile V=99
   ``` 
安装
--- 
本软件包依赖库：ip、ipset、iptables-mod-tproxy、dnsmasq-full、coreutils、coreutils-base64，opkg会自动安装上述库文件

提醒：请停用当前针对域名污染的其他处理软件，不要占用UDP 5353端口

将编译成功的openwrt-brook-tproxy*_all.ipk通过winscp上传到路由器的/tmp目录，执行命令：

   ```bash
   #刷新opkg列表
   opkg update
   
   #删除dnsmasq（第一次安装，如果没有安装dnsmasq-full，需手动卸载dnsmasq，其他情况下不需要）
   opkg remove dnsmasq 
   
   #安装软件包
   opkg install /tmp/openwrt-brook-tproxy*_all.ipk 
   ```
安装后强烈建议重启路由器，因为luci有缓存机制，在升级或新装IPK后，如不重启有时会出现一些莫名其妙的问题；另由于安装、修改、调用dnsmasq-full，安装后最好能重启路由器

配置
---

   软件包通过luci配置， 支持的配置项如下:  
   
   客户端服务器配置：

   键名           | 数据类型   | 说明
   ---------------|------------|-----------------------------------------------
   server         | 主机类型   | 服务器地址, 可以是 IP 或者域名，推荐使用IP地址
   server_port    | 数值       | 服务器端口号, 小于 65535   
   local_port     | 数值       | 本地监听的端口号, 小于 65535
   password       | 字符串     | 服务端设置的密码
   

   客户端其他配置：
   
   名称                        | 含义
   ----------------------------|-----------------------------------------------------------
   全局服务器                  | 选择要连接的Brook 透明代理服务器
   DNS服务器地址和端口         | DNS请求转发的服务器，一般设置为google的dns地址
   访问控制-被忽略IP列表       | IP路由模式时有效，用于指定存放国内IP网段的文件，这些网段不经过代理
   访问控制-额外被忽略IP       | IP路由模式时有效，用于添加额外的不经过代理的目的IP地址
   访问控制-强制走代理IP       | 用于添加需要经过代理的目的IP地址
   内网主机访问控制            | 可以控制内网中哪些IP能走代理，哪些不能走代理，可以指定下面列表内或列表外IP
   内网主机列表                | 内网IP列表，可以指定多个
   
   
   IP路由模式的数据文件为/etc/china_ip.txt,包含国内所有IP网段，一般很少变动，无需更新，如要更新，请在“状态”页面更新
   
   GFW列表模式的数据文件为/etc/dnsmasq.ssr/gfw_list.conf，包含所有被墙网站，如需更新，请在“状态”页面更新
   
   支持广告过滤，数据文件为/etc/dnsmasq.ssr/ad.conf，其原理是将广告网站的IP地址解析为127.0.0.1，使用的数据库为easylistchina+easylist；广告过滤模块缺省未安装，用户在“状态”页面更新广告数据库后自动打开，如打开广告过滤后出现问题，请删除此文件并重启dnsmasq

  [1]: https://github.com/txthinking/brook
  [2]: https://wiki.openwrt.org/doc/howto/obtain.firmware.sdk
