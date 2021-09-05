#
# Copyright (C) 2019 EWSI
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libdcwsocket
PKG_VERSION:=1.1.0
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/ewsi/$(PKG_NAME)/tar.gz/v$(PKG_VERSION)?
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_HASH:=71383c4d8c5f58c1299a3717d7de9a8b5dabfd51a2dcf9993248f2709908d23a

PKG_MAINTAINER:=Carey Sonsino <careys@edgewaterwireless.com>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libdcwsocket
  SECTION:=libs
  CATEGORY:=Libraries
  SUBMENU:=Networking
  TITLE:=Dual-Channel socket library
  URL:=https://www.edgewaterwireless.com
endef

define Package/libdcwsocket/description
  User-land C library for sending and receiving DCW "EtherType"d messages
endef

TARGET_CFLAGS += -std=c89 -ffunction-sections -fdata-sections -flto
TARGET_LDFLAGS += -Wl,--gc-sections,--as-needed

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/$(PKG_NAME)*.so* $(1)/usr/lib/
endef

define Package/libdcwsocket/install
	$(INSTALL_DIR) $(1)/usr/lib
	# Note: $(INSTALL_BIN) does not keep symlinks, so use $(CP)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/$(PKG_NAME)*.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libdcwsocket))
