#
# Copyright (C) 2017-2019 Jian Chang <aa65535@live.com>
#
# Copyright (C) 2021 ImmortalWrt
# <https://immortalwrt.org>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=simple-obfs
PKG_VERSION:=0.0.5
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/shadowsocks/simple-obfs.git
PKG_SOURCE_DATE:=2019-08-17
PKG_SOURCE_VERSION:=486bebd9208539058e57e23a12f23103016e09b4
PKG_MIRROR_HASH:=b1ae62a6826616b35b6aea0c6b2d55b5f36da96be0cd1c229c263b508d1aeef0

PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Jian Chang <aa65535@live.com>

PKG_BUILD_DEPENDS:=libev
PKG_FIXUP:=autoreconf
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/simple-obfs/template
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Simple-obfs
	URL:=https://github.com/shadowsocks/simple-obfs
	DEPENDS:=+libpthread +libev
endef

define Package/simple-obfs-client
	$(call Package/simple-obfs/template)
	TITLE+= (client)
	PROVIDES:=simple-obfs
endef

define Package/simple-obfs-server
	$(call Package/simple-obfs/template)
	TITLE+= (server)
endef

define Package/simple-obfs/description
	Simple-obfs is a simple obfusacting tool, designed as plugin server of shadowsocks.
endef

Package/simple-obfs-client/description = $(Package/simple-obfs/description)
Package/simple-obfs-server/description = $(Package/simple-obfs/description)

CONFIGURE_ARGS += \
	--disable-ssp \
	--disable-documentation \
	--disable-assert

define Package/simple-obfs-client/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/obfs-local $(1)/usr/bin/obfs-local
endef

define Package/simple-obfs-server/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/obfs-server $(1)/usr/bin/obfs-server
endef

$(eval $(call BuildPackage,simple-obfs-client))
$(eval $(call BuildPackage,simple-obfs-server))