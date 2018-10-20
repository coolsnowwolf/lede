#
# Copyright (C) 2010-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=autocore
PKG_VERSION:=1
PKG_RELEASE:=16


include $(INCLUDE_DIR)/package.mk

define Package/autocore
  TITLE:=x86/x64 auto core loadbalance script.
  MAINTAINER:=Lean
  DEPENDS:=@TARGET_x86 +bc +lm-sensors +ethtool
endef

define Package/autocore/description
A usb autoconfig hotplug script.
endef

define Build/Compile
endef

define Package/autocore/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/autocore $(1)/etc/init.d/autocore
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) ./files/index.htm $(1)/etc/index.htm
endef

$(eval $(call BuildPackage,autocore))
