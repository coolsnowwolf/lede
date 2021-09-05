#
# Copyright (C) 2010-2015 OpenWrt.org
# Copyright (C) 2010 segal.di.ubi.pt
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=sshtunnel
PKG_VERSION:=4
PKG_RELEASE:=5
PKG_LICENSE:=GPL-2.0-or-later

PKG_MAINTAINER:=Nuno Goncalves <nunojpg@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/sshtunnel
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=SSH
  TITLE:=Manages Local and Remote openssh ssh(1) tunnels
  DEPENDS:=+openssh-client
  PKGARCH:=all
endef

define Package/sshtunnel/description
Creates openssh ssh(1) Local and Remote tunnels configured in UCI file. Can be used to allow remote connections, possibly over NATed connections or without public IP/DNS
endef

define Package/sshtunnel/conffiles
/etc/config/sshtunnel
endef

define Build/Compile
endef

define Package/sshtunnel/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/sshtunnel.init $(1)/etc/init.d/sshtunnel
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/uci_sshtunnel $(1)/etc/config/sshtunnel
endef

$(eval $(call BuildPackage,sshtunnel))
