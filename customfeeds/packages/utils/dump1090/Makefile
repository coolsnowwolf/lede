#
# Copyright (C) 2013-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dump1090
PKG_VERSION:=4.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/flightaware/dump1090/tar.gz/v${PKG_VERSION}?
PKG_HASH:=49ace5a06da3d51384de83a4199d043c6093ad475909c58317cedece0e9590d1

PKG_MAINTAINER:=Álvaro Fernández Rojas <noltari@gmail.com>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING LICENSE

PKG_BUILD_DEPENDS:=libusb
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/dump1090/Default
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Mode S decoder for the Realtek RTL2832U
  URL:=https://github.com/flightaware/dump1090
endef

define Package/dump1090
  $(call Package/dump1090/Default)
  TITLE+= (dump1090)
  DEPENDS+=+librtlsdr +uhttpd +libncurses
endef

define Package/view1090
  $(call Package/dump1090/Default)
  TITLE+= (view1090)
  DEPENDS+=+libncurses
endef

define Package/dump1090/description
 Dump1090 is a Mode S decoder specifically designed for RTLSDR devices.
endef

define Package/view1090/description
 View1090 is a Mode S messages viewer for dump1090 devices.
endef

MAKE_FLAGS += \
	BLADERF=no \
	CFLAGS="$(TARGET_CFLAGS)" \
	UNAME="Linux"

TARGET_CFLAGS += -ffunction-sections -fdata-sections -flto
TARGET_LDFLAGS += -Wl,--gc-sections,--as-needed

define Package/dump1090/conffiles
/etc/config/dump1090
endef

define Package/dump1090/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) files/dump1090.init $(1)/etc/init.d/dump1090
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) files/dump1090.config $(1)/etc/config/dump1090
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_DATA) files/dump1090.default $(1)/etc/uci-defaults/00_uhttpd_dump1090
	$(INSTALL_DIR) $(1)/usr/share/dump1090
	$(CP) $(PKG_BUILD_DIR)/public_html/* $(1)/usr/share/dump1090
	$(LN) /var/run/dump1090 $(1)/usr/share/dump1090/data
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dump1090 $(1)/usr/bin
endef

define Package/view1090/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/view1090 $(1)/usr/bin
endef

$(eval $(call BuildPackage,dump1090))
$(eval $(call BuildPackage,view1090))
