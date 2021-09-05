#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=banhostlist
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_MAINTAINER:=Matteo Croce <matteo@openwrt.org>
PKG_LICENCE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/banhostlist
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=List of advertising domains
  URL:=http://winhelp2002.mvps.org/
endef

define Package/banhostlist/description
List of domains serving advertising and malicious content to easily block them
endef


define Build/Compile/Default
endef

define Package/banhostlist/install
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_BIN) ./files/updatebanhost $(1)/etc/hotplug.d/iface/80-banhosts
endef

$(eval $(call BuildPackage,banhostlist))
