#
# Copyright (C) 2010-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=autocore
PKG_VERSION:=1
PKG_RELEASE:=7

PKG_MAINTAINER:=Lean

include $(INCLUDE_DIR)/package.mk

define Package/autocore
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=Multicore load-balancing script
	PKGARCH:=all
endef

define Package/autocore/description
 An autoconfig hotplug script that improves network performance by using Receive Packet Steering (RPS) and Receive Flow Steering (RFS).
endef

define Build/Compile
endef

define Package/autocore/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/autocore $(1)/etc/init.d/autocore
endef

$(eval $(call BuildPackage,autocore))
