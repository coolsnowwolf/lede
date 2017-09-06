
# Copyright (C) 2007-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=authsae
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/cozybit/authsae.git
PKG_SOURCE_DATE:=2014-06-09
PKG_SOURCE_VERSION:=8531ab158910a525d4bcbb3ad02c08342f6987f2
PKG_MIRROR_HASH:=a8fbed9eada17c552c70d834ee8f7c1e0987df59839bc113b3ec000500696715

PKG_BUILD_PARALLEL:=1
CMAKE_INSTALL:=1

CMAKE_OPTIONS += -DSYSCONF_INSTALL_DIR=/etc

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>
PKG_LICENSE:=BSD-4-Clause

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/authsae
  SECTION:=net
  CATEGORY:=Network
  TITLE:=80211s mesh security
  DEPENDS:=+libopenssl +libconfig +libnl-tiny +@OPENSSL_WITH_EC
endef

TARGET_CFLAGS += -D_GNU_SOURCE

define Package/authsae/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
	$(INSTALL_DIR) $(1)/lib/wifi
	$(INSTALL_DATA) ./files/lib/wifi/authsae.sh $(1)/lib/wifi/
endef

$(eval $(call BuildPackage,authsae))
