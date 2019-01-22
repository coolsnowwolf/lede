Shadowsocksr-libev for OpenWrt/LEDE
===
版本 2.5.6  2017.6.13破娃酱最后一commit

相比2016-stable分支,有如下不同

1、支持auth_chain_a;

2、ssr-tunnel合并进ssr-local

3、去掉ssr-server


为编译[此固件][O]所需依赖包而写的Makefile,在CC 15.05,CC 15.05.1,LEDE 17.01.1 17.01.2编译成功

简介
---

本项目是 [shadowsocksr-libev][1] 在 OpenWrt 上的移植 ,写法参考https://github.com/shadowsocks/openwrt-shadowsocks/ 
 
可与[luci-app-shadowsocksR][P]搭配使用

 
特性
---

软件包只包含 [shadowsocksr-libev][1] 的可执行文件
可编译两种版本

 - shadowsocksr-libev

   ```
   客户端/
   └── usr/
       └── bin/
           ├── ssrr-local       // 提供 SOCKS 代理和旧版ssr-tunnel的端口转发功能 
           └── ssrr-redir      // 提供透明代理, 从 v2.2.0 开始支持 UDP
   ```



获取最新代码
---
可以修改Makefile中PKG_SOURCE_VERSION为你需要编译的commit id

编译
---

 - 从 OpenWrt 的 [SDK][S] 编译

   ```bash
   # 以 ar71xx 平台为例
   tar xjf OpenWrt-SDK-ar71xx-for-linux-x86_64-gcc-4.8-linaro_uClibc-0.9.33.2.tar.bz2
   cd OpenWrt-SDK-ar71xx-*
   # 安装 feeds
   # 方案一(推荐):使用自定义feeds
     git clone https://github.com/AlexZhuo/openwrt-feeds.git package/feeds
   # 方案二:使用官方feeds
     ./scripts/feeds update base packages
     ./scripts/feeds install zlib libopenssl libpolarssl libmbedtls libpcre
     rm -rf package/feeds/base/mbedtls/patches
   # 获取 shadowsocksr-libev Makefile
   git clone https://github.com/AlexZhuo/openwrt-shadowsocksr.git package/shadowsocksr-libev
   # 选择要编译的包 Network -> shadowsocksr-libev
   make menuconfig
   # 开始编译
   make package/shadowsocksr-libev/compile V=99
   ```

编译错误汇总
---
1、报错`C compiler cannot create executables`

错误原因：当前Linux系统没有完整的编译环境

解决方法：sudo apt-get install ccache build-essential 


2、报错`configure: error: PolarSSL libraries not found.`

错误原因：SDK没有找到PolarSSl的feed

解决方法：git clone https://github.com/AlexZhuo/openwrt-feeds.git package/feeds


3、报错`configure: error: "zlib header files not found."`

错误原因：SDK没有找到libopenssl和zlib的feed，该问题会出现在LEDE rc1的SDK中，使用17.01.0 SDK不会有这个问题

解决方法：git clone https://github.com/AlexZhuo/openwrt-feeds.git package/feeds


4、报错
```
checking for pcre-config... pcre-config
checking for pcre headers in ... not found
checking for library containing pcre_exec... no
configure: error: Cannot find pcre library. Configure --with-pcre=DIR
```
报错原因：SDK没有找到pcre的feed

解决方法：git clone https://github.com/AlexZhuo/openwrt-feeds.git package/feeds或者

./scripts/feeds update base packages

./scripts/feeds install libpcre


5、执行`./scripts/feeds install zlib libopenssl libpolarssl libmbedtls libpcre`时输出不为如下标准输出
```
Installing package 'zlib' from base
Installing package 'openssl' from base
Installing package 'libpolarssl' from base
Installing package 'mbedtls' from base
Installing package 'pcre' from packages
```
而是报找不到feed的错误，如下
```
WARNING: No feed for package 'zlib' found, maybe it's already part of the standard packages?
WARNING: No feed for package 'libopenssl' found, maybe it's already part of the standard packages?
WARNING: No feed for package 'libpolarssl' found, maybe it's already part of the standard packages?
WARNING: No feed for package 'libmbedtls' found, maybe it's already part of the standard packages?
Installing package 'pcre' from packages
```

报错原因：一般出现在LEDE rc1,rc2的SDK上，把17.0.2 LEDE SDK中的feeds.conf.default文件内容复制到SDK中去就可以了。

解决方法：更换SDK，或者修改SDK根目录下的feeds.conf.default文件
```
src-git base https://git.lede-project.org/source.git;v17.01.2
src-git packages https://git.lede-project.org/feed/packages.git^dc558eaa296686603c1730c1aab01f3ea69d7831
src-git luci https://git.lede-project.org/project/luci.git^7f6fc1681f7becc514a58082e871f3855d3a123f
src-git routing https://git.lede-project.org/feed/routing.git^dbbad8472288498c17825303d834da3ee5030806
src-git telephony https://git.lede-project.org/feed/telephony.git^1f0fb2538ba6fc306198fe2a9a4b976d63adb304
```
或

git clone https://github.com/AlexZhuo/openwrt-feeds.git package/feeds


6、报错
```
checking for cipher_init_ctx in -lpolarssl... no
configure: error: PolarSSL libraries not found.
```
报错原因：官方源缺少PolarSSl依赖

解决方法：复制https://github.com/AlexZhuo/openwrt-feeds/tree/master/base/ 中polarssl文件夹到SDK根目录/package/feeds/base

7、报错
```
checking whether mbedtls supports the Camellia block cipher or not... configure: error: MBEDTLS_CAMELLIA_C required
```
报错原因：官方源mbedtls版本太高,应该使用2.4.0

解决方法：复制https://github.com/AlexZhuo/openwrt-feeds/tree/master/base 中mbedtls文件夹替换SDK根目录/package/feeds/base/mbedtls



配置
---

   软件包本身并不包含配置文件, 配置文件内容为 JSON 格式, 支持的键:

   键名           | 数据类型 | 说明
   ---------------|----------|-----------------------------------------------
   server         | 字符串   | 服务器地址, 可以是 IP 或者域名
   server_port    | 整数值   | 服务器端口号
   local_address  | 字符串   | 本地绑定的 IP 地址, 默认 `127.0.0.1`
   local_port     | 整数值   | 本地绑定的端口号
   password       | 字符串   | 服务端设置的密码
   method         | 字符串   | 加密方式, [详情参考][E]
   timeout        | 整数值   | 超时时间（秒）, 默认 60
   fast_open      | 布尔值   | 是否启用 [TCP Fast Open][F], 只适用于 `ss-local`, `ss-server`
   auth           | 布尔值   | 是否启用[一次验证][A]
   nofile         | 整数值   | 设置 Linux ulimit
   protocol       | 字符串   | 混淆协议 [详情][N]
   protocol_param | 字符串   | 协议参数 [详情][N]
   obfs           | 字符串   | 混淆方式 [详情][N]
   obfs_param     | 字符串   | 混淆参数 [详情][N]
   
  [1]: https://github.com/shadowsocksr/shadowsocksr-libev
  [2]: https://bintray.com/aa65535/opkg/shadowsocks-libev/_latestVersion "预编译 IPK 下载"
  [B]: https://api.bintray.com/packages/aa65535/opkg/shadowsocks-libev/images/download.svg
  [3]: https://github.com/shadowsocks/luci-app-shadowsocks
  [A]: https://shadowsocks.org/en/spec/one-time-auth.html
  [E]: https://github.com/shadowsocks/luci-app-shadowsocks/wiki/Encrypt-method
  [F]: https://github.com/shadowsocks/shadowsocks/wiki/TCP-Fast-Open
  [S]: https://wiki.openwrt.org/doc/howto/obtain.firmware.sdk
  [M]: https://www.multipath-tcp.org/
  [N]: https://github.com/breakwa11/shadowsocks-rss/blob/master/ssr.md
  [O]: http://www.right.com.cn/forum/thread-198649-1-1.html
  [P]: https://github.com/AlexZhuo/luci-app-shadowsocksR
