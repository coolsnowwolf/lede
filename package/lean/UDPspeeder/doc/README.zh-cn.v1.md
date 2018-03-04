# UDPspeeder (v1)
![image0](/images/Capture7.PNG)
UDP双边加速工具，降低丢包率，配合vpn可以加速任何协议，尤其适用于加速游戏和网页打开速度；同时也是一个UDP连接的调试和统计工具。

这个是我自己稳定用了一个月的项目，用来加速美服的Brawl Stars和亚服的Mobile Legend，效果不错。加速前卡得几乎没法玩，加速后就没怎么卡过了。

注：目前最新版是v2版，这个是v1版的主页

#### 效果
![image0](/images/Capture8.PNG)
#### 原理简介
目前原理是多倍发包。以后会做各种优化，比如：对高频率的短包先合并再冗余；FEC（Forward Error Correction），在包速低的时候多倍发包，包速高时用FEC(这些功能在v2版里已经实现)。

跟net-speeder比，优势在于client和server会把收到的多余包自动去掉，这个过程对上层透明，没有兼容性问题。而且发出的冗余数据包会做长度和内容的随机化，抓包是看不出发了冗余数据的，所以不用担心vps被封的问题。

每个冗余数据包都是间隔数毫秒（可配置）以后延迟发出的，可以避开中间路由器因为瞬时buffer长度过长而连续丢掉所有副本。

模拟一定的延迟抖动，这样上层应用计算出来的RTT方差会更大，以等待后续冗余包的到达，不至于发生在冗余包到达之前就触发重传的尴尬。

#### 适用场景
绝大部分流量不高的情况。程序本身加速udp，但是配合openvpn可以加速任何流量。网络状况不好时，游戏卡得没法玩，或者网页卡得没法打开，使用起来效果最好。对于解决语音通话的断断续续效果也不错。不适合大流量的场景，比如BT下载和在线看视频。 无论从自己使用效果的角度，还是从国际出口带宽占用的角度，都建议不要在大流量环境使用。

#### 其他功能
输出UDP收发情况报告，可以看出丢包率。

模拟丢包，模拟延迟，模拟jitter。便于通过实验找出应用卡顿的原因。

重复包过滤功能可以关掉，模拟网络本身有重复包的情况。用来测试应用对重复报的支持情况。

client支持多个udp连接，server也支持多个client

目前有amd64,x86,ar71xx,树莓派armv7和android的binary

如果你需要绕过UDP屏蔽/QoS，或者需要连接复用/连接保持功能，或者是加密。解决方案在另一个repo(可以跟UDPspeeder一起使用)：

https://github.com/wangyu-/udp2raw-tunnel

# 简明操作说明

### 环境要求
Linux主机，可以是桌面版，可以是android手机/平板，可以是openwrt路由器，也可以是树莓派。在windows和mac上配合虚拟机可以稳定使用（speeder跑在Linux里，其他应用照常跑在window里，桥接模式测试可用）。

android版需要通过terminal运行。

### 安装
下载编译好的二进制文件，解压到本地和服务器的任意目录。

https://github.com/wangyu-/UDPspeeder/releases

### 运行
假设你有一个server，ip为44.55.66.77，有一个服务监听在udp 7777端口。 假设你需要加速本地到44.55.66.77:7777的流量。
```
在client端运行:
./speeder_ar71xx -l0.0.0.0:3333 -r 44.55.66.77:8855 -c  -d2 -k "passwd"

在server端运行:
./speeder_amd64 -l0.0.0.0:8855 -r127.0.0.1:7777 -s -d2 -k "passwd"
```

现在client和server之间建立起了tunnel。想要连接44.55.66.77:7777，只需要连接 127.0.0.1:3333。来回的所有的udp流量会被加速。

###### 注:

-d2 表示除了本来的包以外，额外再发2个冗余包。可调。

-k 指定一个字符串，server/client间所有收发的包都会被异或，改变协议特征，防止UDPspeeder的协议被运营商针对。

# 进阶操作说明

### 命令选项
```
UDPspeeder
git version:b4bd385e88    build date:Sep 11 2017 10:29:25
repository: https://github.com/wangyu-/UDPspeeder

usage:
    run as client : ./this_program -c -l local_listen_ip:local_port -r server_ip:server_port  [options]
    run as server : ./this_program -s -l server_listen_ip:server_port -r remote_ip:remote_port  [options]

common option,must be same on both sides:
    -k,--key              <string>        key for simple xor encryption,default:"secret key"
main options:
    -d                    <number>        duplicated packet number, -d 0 means no duplicate. default value:0
    -t                    <number>        duplicated packet delay time, unit: 0.1ms,default value:20(2ms)
    -j                    <number>        simulated jitter.randomly delay first packet for 0~jitter_value*0.1 ms,to
                                          create simulated jitter.default value:0.do not use if you dont
                                          know what it means
    --report              <number>        turn on udp send/recv report,and set a time interval for reporting,unit:s
advanced options:
    -t                    tmin:tmax       simliar to -t above,but delay randomly between tmin and tmax
    -j                    jmin:jmax       simliar to -j above,but create jitter randomly between jmin and jmax
    --random-drop         <number>        simulate packet loss ,unit:0.01%
    --disable-filter                      disable duplicate packet filter.
    -m                    <number>        max pending packets,to prevent the program from eating up all your memory,
                                          default value:0(disabled).
other options:
    --log-level           <number>        0:never    1:fatal   2:error   3:warn 
                                          4:info (default)     5:debug   6:trace
    --log-position                        enable file name,function name,line number in log
    --disable-color                       disable log color
    --sock-buf            <number>        buf size for socket,>=10 and <=10240,unit:kbyte,default:1024
    -h,--help                             print this help message

```
### 包发送选项，两端设置可以不同。 只影响本地包发送。
##### -d 选项
设置冗余包数量。
##### -t 选项
为冗余包的发送，增加一个延迟.对中间路由buffer做优化，应对瞬时Buffer过长导致的连续丢包.对于多个冗余包，依次在前一个包的基础上增加这个延迟。
##### -j 选项
为原始数据的发送，增加一个延迟抖动值。这样上层应用计算出来的RTT方差会更大，以等待后续冗余包的到达，不至于发生在冗余包到达之前就触发重传的尴尬。配合-t选项使用。正常情况下跨国网络本身的延迟抖动就很大。可以不用设-j

##### --report  选项
数据发送和接受报告。开启后可以根据此数据推测出包速和丢包率等特征。

##### 加强版 -t 选项
跟普通-t类似，允许设置最大值最小值，用随机延迟发送冗余包。

##### 加强版 -j 选项
允许给jitter选项设置最大值最小值。在这个区间随机化jitter。如果最大值最小值一样就是模拟延迟。可以模拟高延迟、高jitter的网络环境。

##### --random-drop 选项
随机丢包。模拟恶劣的网络环境时使用。

### 包接收选项，两端设置可以不同。只影响本地包接受
##### --disable-filter    
关闭重复包过滤器。这样配合-d 选项可以模拟有重复包的网络环境。

# 应用

#### UDPspeeder + openvpn加速任何流量
如果你只是需要玩游戏，效果预期会kcp/finalspeed方案更好。可以优化tcp游戏的延迟（通过冗余发包，避免了上层的重传）。比如魔兽世界用的是tcp连接。
![image0](/images/Capture2.PNG)

跟openvpn via kcptun方式的对比：

kcptun在udp层有RS code，也是一种冗余传输，通过openvpn把流量转成tcp，再通过kcptun加速是有一定效果的。但是tcp只支持按序到达。按序到达的意思是，如果你发了1 2 3 4 5 6，6个包，如果第一个包丢了，那么必须等第一个包重传成功以后 2 3 4 5 6 才能到达；只要有一个包不到，后续数据包就要一直等待。用tcp承载udp流量会破坏udp的实时性。会造成游戏卡顿更严重。

udp协议本身是ip协议加上了端口之后的直接封装，udp继承了ip协议的实时/乱序到达特性，更适合中转vpn。

#### UDPspeeder + kcptun/finalspeed + $*** 同时加速tcp和udp流量
如果你需要用加速的tcp看视频和下载文件，这样效果比vpn方案更好。不论是速度，还是流量的耗费上。
![image0](/images/cn/speeder_kcptun.PNG)

#### UDPspeeder + openvpn + $*** 混合方案
也是我正在用的方案。优点是可以随时在vpn和$*** 方案间快速切换。
实际部署起来比图中看起来的还要简单。不需要改路由表，需要做的只是用openvpn的ip访问$*** server。

![image0](/images/cn/speeder_vpn_s.PNG)
(也可以把图中的$*** server换成其他的socks5 server，这样连$*** client也不需要了)
# 编译教程
暂时先参考udp2raw的这篇教程，几乎一样的过程。

https://github.com/wangyu-/udp2raw-tunnel/blob/master/doc/build_guide.zh-cn.md
