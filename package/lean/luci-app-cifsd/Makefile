# This is free software, licensed under the Apache License, Version 2.0 .

include $(TOPDIR)/rules.mk

LUCI_TITLE:=Network Shares - CIFSD CIFS/SMB kernel fileserver
LUCI_DEPENDS:=+ksmbd-server
LUCI_PKGARCH:=all
PKG_NAME:=luci-app-cifsd
PKG_VERSION:=1
PKG_RELEASE:=12

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
