#
# Copyright (C) 2010 segal.di.ubi.pt
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=watchcat
PKG_VERSION:=1
PKG_RELEASE:=12

PKG_MAINTAINER:=Roger D <rogerdammit@gmail.com>
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/watchcat
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Enable the configuration of programmed reboots or network interface restarts
  PKGARCH:=all
endef

define Package/watchcat/description
Restart network interfaces or reboot if pings to hosts fail, or set up periodic reboots. Configured via UCI /etc/config/watchcat
endef

define Package/watchcat/conffiles
/etc/config/watchcat
endef

define Build/Compile
endef

define Package/watchcat/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/watchcat.init $(1)/etc/init.d/watchcat
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./files/watchcat.sh $(1)/usr/bin/watchcat.sh
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/watchcat.config $(1)/etc/config/watchcat
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/migrate-watchcat $(1)/etc/uci-defaults/migrate-watchcat
endef

$(eval $(call BuildPackage,watchcat))
