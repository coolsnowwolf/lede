# Easy QoS for OpenWRT/Lede

![](https://img.shields.io/badge/license-GPLV3-brightgreen.svg?style=plastic "License")

![](https://github.com/lwxlwxlwx/blob/master/eqos_zh.png)

# 特性
* 支持基于IP地址限速
* 未使用iptables打MARK
* 提供Luci界面

# 安装到OpenWRT/LEDE
	
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
