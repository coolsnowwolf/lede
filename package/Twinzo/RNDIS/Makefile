#
# Copyright (C) 2018 By-Ameng Openwrt.org
#
# This is free software, licensed under the Apache License, Version 2.0 .
#
#Copyright (C) 2019 Twinzo1 <1282055288@qq.com>

include $(TOPDIR)/rules.mk

LUCI_TITLE:=Remote NDIS(USB sharing Internet) 
LUCI_DEPENDS:=+kmod-usb-net +kmod-usb-net-rndis +kmod-usb-net-cdc-ether +usbutils
LUCI_PKGARCH:=all
PKG_VERSION:=2.0
PKG_RELEASE:=1
PKG_DEPENDS:=+libusb

include $(TOPDIR)/feeds/luci/luci.mk
# call BuildPackage - OpenWrt buildroot signature
