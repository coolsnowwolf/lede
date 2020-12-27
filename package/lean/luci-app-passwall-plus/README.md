# passwall plus  

[![GitHub release](https://img.shields.io/github/release/yiguihai/luci-app-passwall-plus.svg)](https://github.com/yiguihai/luci-app-passwall-plus/releases/latest)
[![GitHub downloads](https://img.shields.io/github/downloads/yiguihai/luci-app-passwall-plus/latest/total.svg)](https://github.com/yiguihai/luci-app-passwall-plus/releases/latest)  

## 功能特点

- 默认直连，撞墙自动切换服务器
- 多线服务器流量转发负载均衡
- 按域名分流解决地区版权问题
- 自动选择最优线路(本地路由)
- 自动选择最优dns返回(本地路由)

### 问与答
为什么只有一些常用的协议设置？
> 因为这个项目本意是类似于流量转发器的目标，所以没有添加其它协议。你可以自己开一个本地代理然后在本插件的"服务器"选项卡中添加合适的协议、地址与监听端口即可。

为什么谷歌市场无法下载应用程序？
> 因为走了udp的5228端口流量。目前本插件无法处理udp流量，等待上游tcproute2支持udp代理转发解决...

安装ipk就可以直接使用了吗?
> 是的，除了all版需要编译 [ipt2socks](https://github.com/zfl9/ipt2socks) 、 [TcpRoute2](https://github.com/GameXG/TcpRoute2) 、 [SmartDNS](https://github.com/pymumu/smartdns) 并安装，其它版本直接安装即可使用。因为云编译已经将所需二进制文件打包进ipk里面了。 

等待时间填多少合适?
> 除了直连组不用设置其他穿墙服务器组都要设置，视网速带宽而定，过高国外会慢过低会全部走代理建议在 大于125 小于200

为什么要设置一组国外的dns?
> 因为查询一些特殊网址电信运营商会返回127.0.0.1或者0.0.0.0的地址进行投毒污染，造成无法使用tcproute2的强制纠正dns的功能。经作者测试国外dns不会返回私网地址。

域名白名单有什么用？
> 可以指定域名走哪一个代理，比如github没有被墙但是国内都解析到新加坡的ip地址造成网速过慢，你可以指定走那一个出国代理达到高速访问的体验。还可以用来破解视频网站的地区版权验证。

### 预览图
<img src="/view/1.jpg" alt="展示图" title="查看图片" width="200" height="250" /><img src="/view/2.png" alt="展示图" title="查看图片" width="200" height="250" />

