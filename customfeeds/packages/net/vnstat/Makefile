#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=vnstat
PKG_VERSION:=1.18
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://humdi.net/vnstat
PKG_HASH:=d7193592b9e7445fa5cbe8af7d3b39982f165ee8fc58041ff41f509b37c687d5
PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>

PKG_INSTALL:=1

PKG_BUILD_DEPENDS:=libgd
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/vnstat/Default
  SECTION:=net
  CATEGORY:=Network
  URL:=http://humdi.net/vnstat/
endef

define Package/vnstat
$(call Package/vnstat/Default)
  TITLE:=Console-based network traffic monitor
endef

define Package/vnstat/description
	vnStat is a network traffic monitor for Linux that keeps a log of daily
	network traffic for the selected interface(s). vnStat isn't a packet
	sniffer. The traffic information is analyzed from the /proc -filesystem,
	so vnStat can be used without root permissions.
endef

define Package/vnstati
$(call Package/vnstat/Default)
  DEPENDS+=vnstat +libgd
  TITLE:=PNG image output support for vnStat
endef

define Package/vnstati/description
	The purpose of vnstati is to provide image output support for statistics
	collected using vnstat(1). However, the image file format is limited to
	png. All basic outputs of vnStat are supported excluding live traffic
	features. The image can be outputted either to a file or to standard
	output.
endef

define Package/vnstat/conffiles
/etc/vnstat.conf
/etc/config/vnstat
endef

define Build/Compile/vnstat
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef

define Build/Compile/vnstati
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include" \
		LDFLAGS="$(TARGET_LDFLAGS) -Wl,-rpath-link,$(STAGING_DIR)/usr/lib" \
		all
endef

define Build/Compile
$(call Build/Compile/vnstat)
$(call Build/Compile/vnstati)
endef

define Package/vnstat/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/vnstat $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/vnstatd $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/cfg/vnstat.conf $(1)/etc/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/vnstat.config $(1)/etc/config/vnstat
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/vnstat.init $(1)/etc/init.d/vnstat
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/vnstat.defaults $(1)/etc/uci-defaults/vnstat
endef

define Package/vnstati/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/vnstati $(1)/usr/bin/
endef

define Package/vnstat/postinst
#!/bin/sh
[ -n "$${IPKG_INSTROOT}" ] || (. /etc/uci-defaults/vnstat) && rm -f /etc/uci-defaults/vnstat
exit 0
endef

$(eval $(call BuildPackage,vnstat))
$(eval $(call BuildPackage,vnstati))
