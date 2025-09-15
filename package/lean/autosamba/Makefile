#
# Copyright (C) 2010-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=autosamba
PKG_VERSION:=1
PKG_RELEASE:=15
PKG_ARCH:=all

PKG_CONFIG_DEPENDS:= \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_KSMBD \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_SAMBA3 \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_SAMBA4 \

include $(INCLUDE_DIR)/package.mk

define Package/autosamba
  TITLE:=Samba autoconfig hotplug script.
  MAINTAINER:=Lean
  DEPENDS:=+wsdd2 +PACKAGE_$(PKG_NAME)_INCLUDE_KSMBD:luci-app-ksmbd +PACKAGE_$(PKG_NAME)_INCLUDE_SAMBA3:luci-app-samba +PACKAGE_$(PKG_NAME)_INCLUDE_SAMBA4:luci-app-samba4 
endef

define Package/autosamba/description
A hotplug script to config Samba share automatically.
endef

define Package/$(PKG_NAME)/config
choice
	prompt "Samba Server Selection"
	default PACKAGE_$(PKG_NAME)_INCLUDE_KSMBD
	
	config PACKAGE_$(PKG_NAME)_INCLUDE_KSMBD
	bool "KSMBD"

	config PACKAGE_$(PKG_NAME)_INCLUDE_SAMBA3
	bool "SAMBA 3"

	config PACKAGE_$(PKG_NAME)_INCLUDE_SAMBA4
	bool "SAMBA 4"
endchoice

endef

define Build/Compile
endef

define Package/autosamba/install
	$(INSTALL_DIR) $(1)/etc/hotplug.d/block
	$(INSTALL_BIN) ./files/20-smb $(1)/etc/hotplug.d/block/20-smb
endef

$(eval $(call BuildPackage,autosamba))
