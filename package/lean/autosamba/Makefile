#
# Copyright (C) 2010-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=autosamba
PKG_VERSION:=1
PKG_RELEASE:=13
PKG_ARCH:=all

include $(INCLUDE_DIR)/package.mk

define Package/autosamba
  TITLE:=Samba autoconfig hotplug script.
  MAINTAINER:=Lean
  DEPENDS:=+luci-app-samba4 +wsdd2
endef

define Package/autosamba/description
A hotplug script to config Samba share automatically.
endef

define Build/Compile
endef

define Package/autosamba/install
	$(INSTALL_DIR) $(1)/etc/hotplug.d/block
	$(INSTALL_BIN) ./files/20-smb $(1)/etc/hotplug.d/block/20-smb
endef

$(eval $(call BuildPackage,autosamba))
