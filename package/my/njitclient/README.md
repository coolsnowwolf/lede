njit8021xclient for iNode C0407
===
唔，这个版本也支持inode 7.0中的 h3c-AES-MD5 算法

See ReadMe.html for more details...

Install
---
仅以debian/ubuntu为例：

1. 安装依赖及编译工具链
    `sudo apt-get install autoconf libtool libpcap-dev libssl-dev pkg-config`
2. 访问网页 https://github.com/bitdust/njit8021xclient/tree/master
3. 点击右侧 Download ZIP 下载源码包
4. 解压到某一目录（假设为 `~/njit8021xclient-master`)
5. 进入目录  `cd ~/njit8021xclient-master`
6. 运行 `autoreconf --install`
7. 运行 `./configure`
8. 运行 `make`
9. 运行 `sudo make install`

完成安装。

Usage
---
1. 运行 `sudo njit-client [用户名]  [密码]  [网卡名]` 即可。（原版使用方法说明请参照README.html文件）
    注意：网卡名为连接校园网的网卡名，一般为 eth0，具体请使用 ifconfig 查找。
2. 程序会输出一些调试信息，其中大部分信息不用理会，
    只需关注如果包括`[*] Server: Success.`这样一行提示即为802.1X认证成功。
3. 如果拨号正常，但是无法上网，请另外打开一个终端，运行 `dhclient [网卡名]` 手动获取IP地址。
4. 实在有问题，重启下试试。

Fork from
---
https://github.com/tengattack/8021xclient-for-windows

https://github.com/liuqun/njit8021xclient

Depends on
---
[libssl](https://wiki.openssl.org/)

[libpcap](http://www.tcpdump.org/)

More info
---
[南京工程学院 - 校园网802.1X客户端 - 项目文档](./documents/Documents.html)

[关于inode7.0中基于AES128的验证算法](./documents/h3c_AES_MD5.md)

