#!/bin/bash
#=================================================
# Description: DIY script
# Lisence: MIT
# Author: P3TERX
# Blog: https://p3terx.com
#=================================================
# Modify default IP
sed -i 's/192.168.1.1/192.168.199.10/g' package/base-files/files/bin/config_generate

#拦截广告的app

cd package
mkdir openwrt-packages
cd openwrt-packages
git clone https://github.com/Leo-Jo/luci-app-koolproxyR.git
