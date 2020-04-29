#
# Copyright (C) 2018 By-Ameng Openwrt.org
#
# This is free software, licensed under the Apache License, Version 2.0 .
#
#Copyright (C) 2019 Twinzo1 <1282055288@qq.com>

include $(TOPDIR)/rules.mk

LUCI_TITLE:=4G(4G denpendency) 
LUCI_DEPENDS:=+kmod-usb-net +kmod-usb-net-cdc-mbim +kmod-usb-net-cdc-subset \
		+luci-proto-3g +luci-proto-qmi \
		+kmod-usb-ohci +kmod-usb-serial +kmod-usb-serial-option \
		+kmod-usb-uhci +kmod-usb2 +comgt-ncm +usb-modeswitch
LUCI_PKGARCH:=all
PKG_VERSION:=1.0
PKG_RELEASE:=1
PKG_DEPENDS:=

include $(TOPDIR)/feeds/luci/luci.mk
# call BuildPackage - OpenWrt buildroot signature
