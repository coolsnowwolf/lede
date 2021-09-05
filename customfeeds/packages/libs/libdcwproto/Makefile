#
# Copyright (C) 2019 EWSI
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libdcwproto
PKG_VERSION:=1.1.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/ewsi/$(PKG_NAME)/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=b3d12f2533eafbb293bbf27608ff39520508d955a084f33894c594f39d2f7c8e

PKG_MAINTAINER:=Carey Sonsino <careys@edgewaterwireless.com>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libdcwproto
  SECTION:=libs
  CATEGORY:=Libraries
  SUBMENU:=Networking
  TITLE:=Dual-Channel WiFi messaging library
  URL:=https://www.edgewaterwireless.com
  DEPENDS:=+kmod-macremapper
endef

define Package/libdcwproto/description
  Platform-independent C library for marshaling and serializing DCW messages
endef

TARGET_CFLAGS += -ffunction-sections -fdata-sections -flto
TARGET_LDFLAGS += -Wl,--gc-sections,--as-needed

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/$(PKG_NAME)*.so* $(1)/usr/lib/
endef

define Package/libdcwproto/install
	$(INSTALL_DIR) $(1)/usr/lib
	# Note: $(INSTALL_BIN) does not keep symlinks, so use $(CP)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/$(PKG_NAME)*.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libdcwproto))
