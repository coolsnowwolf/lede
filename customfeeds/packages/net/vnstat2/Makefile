#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=vnstat2
PKG_VERSION:=2.7
PKG_RELEASE:=1

PKG_SOURCE:=vnstat-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://humdi.net/vnstat
PKG_HASH:=4c28a1d8bc03c2b6e7ab96c876e07dd8ea174b7cad73b7190ecb2b9501e83e9e

PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=COPYING
PKG_MAINTAINER:=Jan Hoffmann <jan@3e8.eu>

PKG_BUILD_DIR:=$(BUILD_DIR)/vnstat-${PKG_VERSION}

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/vnstat2/Default
  SECTION:=net
  CATEGORY:=Network
  URL:=https://humdi.net/vnstat/
  DEPENDS:=+libsqlite3
  CONFLICTS:=vnstat
endef

define Package/vnstat2
$(call Package/vnstat2/Default)
  TITLE:=Console-based network traffic monitor
endef

define Package/vnstat2/description
	vnStat is a network traffic monitor for Linux that keeps a log of daily
	network traffic for the selected interface(s). vnStat isn't a packet
	sniffer. The traffic information is analyzed from the /proc -filesystem,
	so vnStat can be used without root permissions.
endef

define Package/vnstati2
$(call Package/vnstat2/Default)
  DEPENDS+=vnstat2 +libgd
  TITLE:=PNG image output support for vnStat
endef

define Package/vnstati2/description
	The purpose of vnstati is to provide image output support for statistics
	collected using vnstat. However, the image file format is limited to
	png. All basic outputs of vnStat are supported excluding live traffic
	features. The image can be outputted either to a file or to standard
	output.
endef

define Package/vnstat2/conffiles
/etc/vnstat.conf
/etc/config/vnstat
endef

CONFIGURE_ARGS += --disable-extra-paths

define Package/vnstat2/install
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
endef

define Package/vnstati2/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/vnstati $(1)/usr/bin/
endef

$(eval $(call BuildPackage,vnstat2))
$(eval $(call BuildPackage,vnstati2))
