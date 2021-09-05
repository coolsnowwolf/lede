#
# Copyright (c) 2018 Tony Ambardar
# This is free software, licensed under the GNU General Public License v2.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=speedtest-netperf
PKG_VERSION:=1.0.0
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0
PKG_MAINTAINER:=Tony Ambardar <itugrok@yahoo.com>

include $(INCLUDE_DIR)/package.mk

define Package/speedtest-netperf
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Script to measure the performance of your network and router
	DEPENDS:=+netperf
	CONFLICTS:=speedtest
	PKGARCH:=all
endef

define Package/speedtest-netperf/description
Script to measure the performance of your network and router.

Please see https://github.com/openwrt/packages/blob/master/net/speedtest-netperf/files/README.md for further information.
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/speedtest-netperf/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./files/speedtest-netperf.sh $(1)/usr/bin/
endef

$(eval $(call BuildPackage,speedtest-netperf))
