#
# Copyright (C) 2014-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=cshark
PKG_SOURCE_DATE:=2020-07-22
PKG_SOURCE_VERSION:=c0d32fb6df3cd095fecac8aefe8d188170246403
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/cloudshark/cshark.git
PKG_MIRROR_HASH:=35fd177653725e64c330fca62e674c1106d5893c8a33bad29b8e6dac4a4d4dcc

PKG_MAINTAINER:=Luka Perkov <luka@openwrt.org>
PKG_LICENSE:=BSD-2-Clause

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/cshark
  SECTION:=net
  CATEGORY:=Network
  TITLE:=CloudShark capture tool
  URL:=https://cloudshark.io/
  DEPENDS:=+libjson-c +libpcap +libuci +libubox +libuclient +libustream-mbedtls
endef

CMAKE_OPTIONS += \
	-DWITH_DEBUG=OFF

define Package/cshark/conffiles
/etc/config/cshark
endef

define Package/cshark/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/cshark \
		$(1)/sbin/

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) \
		$(PKG_BUILD_DIR)/config/cshark \
		$(1)/etc/config/
endef


$(eval $(call BuildPackage,cshark))
