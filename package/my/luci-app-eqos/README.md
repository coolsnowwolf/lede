# Easy QoS for OpenWRT/Lede([中文](https://github.com/lwxlwxlwx/eqos/blob/master/README_ZH.md))

![](https://img.shields.io/badge/license-GPLV3-brightgreen.svg?style=plastic "License")

![](https://github.com/lwxlwxlwx/blob/master/eqos.png)

# Features
* Support speed limit based on IP address
* No marking by iptables
* Support LuCI interface

# Install to OpenWRT/LEDE
	
	git clone https://github.com/lwxlwxlwx/eqos.git
	cp -r eqos LEDE_DIR/package/eqos
	
	cd LEDE_DIR
	./scripts/feeds update -a
	./scripts/feeds install -a
	
	make menuconfig
	LuCI  --->
		1. Collections  --->
			<*> luci
		3. Applications  --->
			<*> luci-app-eqos...................................... EQOS - LuCI interface
		4. Themes  --->
			<*> luci-theme-material
	Network  --->
		-*- eqos................... Easy QoS(Support speed limit based on IP address)
	
	make package/eqos/compile V=s
