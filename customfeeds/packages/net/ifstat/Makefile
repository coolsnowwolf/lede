#
# Copyright (C) 2016 Nikil Mehta <nikil.mehta@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ifstat
PKG_VERSION:=1.1
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://gael.roualland.free.fr/ifstat/
PKG_HASH:=8599063b7c398f9cfef7a9ec699659b25b1c14d2bc0f535aed05ce32b7d9f507

PKG_MAINTAINER:=Nikil Mehta <nikil.mehta@gmail.com>
PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk

define Package/ifstat
  SECTION:=net
  CATEGORY:=Network
  TITLE:=InterFace STATistics Monitoring
  URL:=http://gael.roualland.free.fr/ifstat/
  DEPENDS:=+IFSTAT_SNMP:libnetsnmp
endef

define Package/ifstat/description
	ifstat is a tool to report network interfaces bandwidth just like
	vmstat/iostat do for other system counters. It can monitor local
	interfaces by polling the kernel counters, or remote hosts
	interfaces using SNMP.
endef

define Package/ifstat/config
	source "$(SOURCE)/Config.in"
endef

CONFIGURE_ARGS += --with$(if $(CONFIG_IFSTAT_SNMP),,out)-snmp

define Package/ifstat/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ifstat $(1)/usr/bin/
endef

$(eval $(call BuildPackage,ifstat))
