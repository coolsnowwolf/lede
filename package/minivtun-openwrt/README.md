## minivtun-openwrt

A fast secure and reliable VPN service in non-standard protocol for rapidly deploying VPN servers/clients or getting through firewall. [minivtun](https://github.com/rssnsj/minivtun) was created by [@rssnsj](https://github.com/rssnsj). 

It's a very simple point-to-point tunnel client/server. only less than 20kB size.

This repo is an unoffical port for openwrt, if you prefer the offical one, please visit [minivtun-tools](https://github.com/rssnsj/network-feeds/tree/master/minivtun-tools).

### For Linux

Show you the Offical compile guide below

Install devel libs

	# ubuntu
	sudo apt-get install build-essential libssl-dev
	# CentOS
	sudo yum install make gcc openssl-devel

Compile and install

    git clone https://github.com/rssnsj/minivtun.git minivtun
    cd minivtun/src
    make
    sudo make install

Run and listen(my script copied from shadowvpn, not offical)

	# modify your listenig port and password, etc
	cd minivtun/linux-server
	vi run.sh

	# use bash to run, not sh
	bash run.sh
	
Enjoy it!

### Complie for Openwrt (Client-side)

	# ar71xx platform
	tar xjf OpenWrt-SDK-ar71xx-for-linux-x86_64-gcc-4.8-linaro_uClibc-0.9.33.2.tar.bz2
	cd OpenWrt-SDK-ar71xx-*
	cd openwrt
	git clone https://github.com/lixingcong/minivtun-openwrt package/minivtun-openwrt

	# Select Network -> minivtun
	make menuconfig
	make package/minivtun-openwrt/compile V=99

### Luci-app

A luci-app-minivtun was available: [luci-app-minivtun](https://github.com/lixingcong/luci-app-minivtun).

### Wiki

Please visit offical page [minivtun](https://github.com/rssnsj/minivtun).

### License

GPLv3
