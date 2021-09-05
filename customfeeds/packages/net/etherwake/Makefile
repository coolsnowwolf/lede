#
# Copyright (C) 2007-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=etherwake
PKG_VERSION:=1.09
PKG_RELEASE:=5

PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).orig.tar.gz
PKG_SOURCE_URL:=http://ftp.debian.org/debian/pool/main/e/etherwake
PKG_HASH:=54241c7689579dc86e29e6afbc6d60e69f97135091a1395c8a10f6d5a2daec1d

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION).orig

include $(INCLUDE_DIR)/package.mk

define Package/etherwake
  SECTION:=net
  CATEGORY:=Network
  TITLE:=WoL client for magic packets via ethernet frames
  URL:=http://ftp.debian.org/debian/pool/main/e/etherwake
  MAINTAINER:=Peter Wagner <tripolar@gmx.at>
endef

define Package/etherwake/description
	You can wake up WOL compliant Computers which have been powered down to
	sleep mode or start WOL compliant Computers with a BIOS feature.
	WOL is an abbreviation for Wake-on-LAN. It is a standard that allows you
	to turn on a computer from another location over a network connection.
	ether-wake also supports WOL passwords.
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) -D__UCLIBC__ $(TARGET_LDFLAGS) $(PKG_BUILD_DIR)/ether-wake.c -o $(PKG_BUILD_DIR)/etherwake
endef

define Package/etherwake/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/etherwake $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) files/$(PKG_NAME).config $(1)/etc/config/$(PKG_NAME)
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)
endef

define Package/etherwake/conffiles
/etc/config/etherwake
endef

$(eval $(call BuildPackage,etherwake))
