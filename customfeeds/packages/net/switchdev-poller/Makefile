#
# Copyright (C) 2018-2019 Luiz Angelo Daros de Luca
#
# This is free software, licensed under the GNU General Public License v2.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=switchdev-poller
PKG_VERSION:=1.1.0
PKG_RELEASE:=1
PKG_MAINTAINER:=Luiz Angelo Daros de Luca <luizluca@gmail.com>
PKG_LICENSE:=GPL-2.0-or-later

include $(INCLUDE_DIR)/package.mk

define Package/switchdev-poller
   SECTION:=net
   CATEGORY:=Network
   TITLE:=Poll switchdev ports to bring CPU ports up/down
   DEPENDS:=+swconfig
   PKGARCH:=all
endef

define Package/switchdev-poller/description
  This service monitors switchdev ports and brings down CPU ports when
  all related non-CPU vlan ports are also down. Otherwise, it brings
  the port up.
endef

define Build/Compile
endef

define Package/switchdev-poller/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/switchdev-poller.init $(1)/etc/init.d/switchdev-poller
	$(INSTALL_DIR) $(1)/usr/lib/switchdev-poller
	$(INSTALL_BIN) ./files/switchdev-poller $(1)/usr/lib/switchdev-poller/switchdev-poller
endef

$(eval $(call BuildPackage,switchdev-poller))
