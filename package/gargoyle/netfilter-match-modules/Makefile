#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
PKG_NAME:=netfilter-match-modules
PKG_VERSION:=1.0
PKG_RELEASE:=

PKG_LICENSE:=AGPL-3.0
PKG_LICENSE_FILES:=LICENSE

#PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/netfilter-match-modules
  SECTION:=net
  CATEGORY:=Network
  TITLE:=netfilter-match-modules for openwrt.
endef

define Package/$PKG_NAME/description
	netfilter-match-modules is planter from gargoyle.
endef

define Package/Compile
	sh ./*.sh . . 1 0 >/dev/null 2>&1
endef

$(eval $(call BuildPackage,$PKG_NAME))
