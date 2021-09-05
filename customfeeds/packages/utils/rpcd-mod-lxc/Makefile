#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=rpcd-mod-lxc
PKG_RELEASE=20201208

PKG_LICENSE:=ISC

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/rpcd-mod-lxc
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=LXC rpcd module
  DEPENDS:=+rpcd +liblxc
  MAINTAINER:=Luka Perkov <luka@openwrt.org>
endef

define Build/Prepare
	$(CP) ./files/* $(PKG_BUILD_DIR)/
endef

define Package/rpcd-mod-lxc/install
	$(INSTALL_DIR) $(1)/usr/lib/rpcd
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lib/lxc.so $(1)/usr/lib/rpcd/
endef

define Package/rpcd-mod-lxc/postinst
#!/bin/sh
[ -n "$$IPKG_INSTROOT" ] || /etc/init.d/rpcd reload
endef

$(eval $(call BuildPackage,rpcd-mod-lxc))
