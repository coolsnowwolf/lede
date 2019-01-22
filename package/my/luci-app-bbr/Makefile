#-- Copyright (C) 2018 dz <dingzhong110@gmail.com>

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI support for BBR
LUCI_DEPENDS:=@LINUX_4_9 +kmod-tcp-bbr
LUCI_PKGARCH:=all
PKG_VERSION:=1.12
PKG_RELEASE:=1

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature

