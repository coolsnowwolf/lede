#
# Copyright (C) 2021 ImmortalWrt
# <https://immortalwrt.org>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dns2socks
PKG_VERSION:=2.1
PKG_RELEASE:=2

PKG_SOURCE:=SourceCode.zip
PKG_SOURCE_URL:=@SF/dns2socks
PKG_SOURCE_DATE:=2020-02-18
PKG_HASH:=406b5003523577d39da66767adfe54f7af9b701374363729386f32f6a3a995f4

PKG_MAINTAINER:=ghostmaker
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILE:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/dns2socks
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=IP Addresses and Names
  TITLE:=The utility to resolve DNS requests via a SOCKS5 tunnel.
  URL:=http://dns2socks.sourceforge.net/
  MAINTAINER:=ghostmaker
  DEPENDS:=+libpthread
endef

define Package/dns2socks/description
  This is a utility to resolve DNS requests via a SOCKS5 tunnel and caches the answers.
endef

UNZIP_CMD:=unzip -q -d $(PKG_BUILD_DIR) $(DL_DIR)/$(PKG_SOURCE)

define Build/Compile
	$(TARGET_CC) \
	$(TARGET_CFLAGS) \
	$(TARGET_CPPFLAGS) \
	$(FPIC) \
	-o $(PKG_BUILD_DIR)/DNS2SOCKS/dns2socks \
	$(PKG_BUILD_DIR)/DNS2SOCKS/DNS2SOCKS.c \
	$(TARGET_LDFLAGS) -pthread
endef

define Package/dns2socks/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/DNS2SOCKS/dns2socks $(1)/usr/bin/dns2socks
endef

$(eval $(call BuildPackage,dns2socks))