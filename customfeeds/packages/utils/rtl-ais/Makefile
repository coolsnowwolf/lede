#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=rtl-ais
PKG_VERSION:=0.3
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/dgiardini/rtl-ais/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=01e2b675226ec403c409cec8b55999008f5c7aa9e82d6c0ba085ef13b200ceb1

PKG_MAINTAINER:=Nuno Goncalves <nunojpg@gmail.com>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk

define Package/rtl-ais
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=rtl-sdr AIS dual channel tuner
  DEPENDS:=+libpthread +librtlsdr
  URL:=https://github.com/dgiardini/rtl-ais
endef

define Package/rtl-ais/description
  rtl_ais uses a rtl2832u dvb-t dongle to tune AIS (Automatic Identification
  System) used on ships and by vessel traffic services
endef

define Package/rtl-ais/conffiles
/etc/config/rtl_ais
endef

TARGET_CFLAGS += -std=gnu89

define Package/rtl-ais/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/rtl_ais $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/rtl_ais.init $(1)/etc/init.d/rtl_ais
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/uci_rtl_ais $(1)/etc/config/rtl_ais
endef

$(eval $(call BuildPackage,rtl-ais))
