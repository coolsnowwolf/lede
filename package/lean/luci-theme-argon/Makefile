#
# Copyright (C) 2008-2019 Jerrykuku
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=Argon Theme
LUCI_DEPENDS:=
PKG_VERSION:=2.2.1
PKG_RELEASE:=20200725

include $(TOPDIR)/feeds/luci/luci.mk

define Package/luci-theme-argon/postinst
#!/bin/sh
[ -f /usr/lib/lua/luci/view/themes/argon/out_header_login.htm ] && mv -f /usr/lib/lua/luci/view/themes/argon/out_header_login.htm /usr/lib/lua/luci/view/header_login.htm
rm -Rf /var/luci-modulecache
rm -Rf /var/luci-indexcache
exit 0
endef

# call BuildPackage - OpenWrt buildroot signature