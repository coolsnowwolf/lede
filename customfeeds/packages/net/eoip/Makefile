#
# Copyright (C) 2020 Bogdan Shatik <bogdikxxx@mail.ru>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=eoip
PKG_VERSION:=0.5
PKG_RELEASE:=1
PKG_MAINTAINER:=Bogdan Shatik <bogdikxxx@mail.ru>
PKG_LICENSE:=GPL-3.0-or-later
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/bogdik/openwrt-linux-eoip/releases/download/0.5/
PKG_HASH:=22f6f3864665adef26c7fbd57543a396108ba2dff1282af8143f18bc2a9912f8

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/eoip
  TITLE:=Mikrotik-compatible userspace EoIP tunnel
  URL:=https://code.google.com/p/linux-eoip/
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=VPN
  DEPENDS:=+kmod-tun +libpthread +liblzo
endef

define Package/eoip/conffiles
/etc/config/eoip
endef

define Package/eoip/description
  linux-eoip can create ethernet tunnels compatible with Mikrotik EoIP tunnel.
  At current moment it is easiest way to create stateless tunnel with Mikrotik.
endef

CONFIGURE_ARGS += \
	--with-kernel="$(LINUX_DIR)" \
	--with-zlib="$(STAGING_DIR)/usr" \
	--with-lzo-include="$(STAGING_DIR)/usr/include/lzo"

define Package/eoip/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/eoip $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/eoip.config $(1)/etc/config/eoip
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/eoip.init $(1)/etc/init.d/eoip
endef

$(eval $(call BuildPackage,eoip))
