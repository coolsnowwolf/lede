#
# Copyright 2019-2023 sirpdboy 
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk
THEME_NAME:=kucat
THEME_TITLE:=Kucat Theme

PKG_NAME:=luci-theme-$(THEME_NAME)
LUCI_TITLE:=Kucat by sirpdboy
LUCI_DEPENDS:= +curl
PKG_VERSION:=1.2.6
PKG_RELEASE:=20230416

include $(TOPDIR)/feeds/luci/luci.mk

define Package/luci-theme-$(THEME_NAME)/postinst
#!/bin/sh

rm -Rf /var/luci-modulecache
rm -Rf /var/luci-indexcache
exit 0

endef
# call BuildPackage - OpenWrt buildroot signature
