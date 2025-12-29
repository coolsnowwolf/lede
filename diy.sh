#!/bin/bash
# diy.sh - 自定义编译配置
cd openwrt

# 1. 添加easymesh插件源（必加，否则找不到easymesh插件）
sed -i '$a src-git easymesh https://github.com/ntlf9t/luci-app-easymesh' feeds.conf.default

# 2. 适配K2P A1双频WiFi驱动（MT7603E/MT7615D）
sed -i 's/^#CONFIG_PACKAGE_kmod-mt7603e/CONFIG_PACKAGE_kmod-mt7603e/' .config
sed -i 's/^#CONFIG_PACKAGE_kmod-mt7615e/CONFIG_PACKAGE_kmod-mt7615e/' .config
sed -i 's/^#CONFIG_PACKAGE_kmod-mt7615d-firmware/CONFIG_PACKAGE_kmod-mt7615d-firmware/' .config

# 3. 开启KVR漫游协议（hostapd关键配置）
sed -i 's/^#CONFIG_PACKAGE_hostapd-utils_KVR/CONFIG_PACKAGE_hostapd-utils_KVR/' .config
sed -i 's/^#CONFIG_PACKAGE_hostapd_KVR/CONFIG_PACKAGE_hostapd_KVR/' .config

# 4. 调整固件大小（适配K2P A1分区，避免超容）
sed -i 's/CONFIG_TARGET_ROOTFS_PARTSIZE=.*/CONFIG_TARGET_ROOTFS_PARTSIZE=128/' .config
