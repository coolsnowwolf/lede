#
# Copyright (C) 2017-2018 Daniel Engberg <daniel.engberg.lists@pyret.net>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=inadyn
PKG_VERSION:=2.7
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://github.com/troglobit/inadyn/releases/download/v$(PKG_VERSION)
PKG_HASH:=eb03bc9d9c09dfbbc651b43a2eb5a967d0454a8293576df23784710dac50c6a4

PKG_MAINTAINER:=
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/inadyn
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+confuse +libopenssl +ca-certificates
  TITLE:=A Dynamic DNS client with SSL/TLS support
  URL:=http://troglobit.com/project/inadyn/
  SUBMENU:=IP Addresses and Names
endef

define Package/inadyn/description
  Inadyn is a small and simple Dynamic DNS, DDNS, client with HTTPS support
endef

define Package/inadyn/conffiles
/etc/inadyn.conf
endef

CONFIGURE_ARGS += \
	--enable-shared \
	--disable-static \
	--enable-openssl \
	--with-pic

define Package/inadyn/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/inadyn $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,inadyn))
