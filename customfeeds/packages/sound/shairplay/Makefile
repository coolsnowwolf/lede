#
# Copyright (C) 2014-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=shairplay
PKG_SOURCE_DATE:=2018-08-24
PKG_SOURCE_VERSION:=096b61ad14c90169f438e690d096e3fcf87e504e
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_SOURCE_DATE).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/juhovh/shairplay/tar.gz/$(PKG_SOURCE_VERSION)?
PKG_HASH:=7e2b013ffe75ea2f13fb12b1aa38b8e2e8b1899ac292d57f05d7b352a3a181cf
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_SOURCE_VERSION)

PKG_MAINTAINER:=Álvaro Fernández Rojas <noltari@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_FIXUP:=autoreconf
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/shairplay
  SECTION:=sound
  CATEGORY:=Sound
  DEPENDS:=+libao +libavahi-compat-libdnssd +libltdl +libpthread
  TITLE:=Shairplay
endef

define Package/shairplay/description
  Free portable AirPlay server implementation similar to ShairPort.
endef

define Package/shairplay/conffiles
/etc/config/shairplay
endef

define Package/shairplay/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/shairplay $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/share/shairplay
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/airport.key $(1)/usr/share/shairplay/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) files/shairplay.init $(1)/etc/init.d/shairplay
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) files/shairplay.config $(1)/etc/config/shairplay
endef

$(eval $(call BuildPackage,shairplay))
