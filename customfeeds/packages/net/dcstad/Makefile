#
# Copyright (C) 2019 EWSI
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dcstad
PKG_VERSION:=1.1.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/ewsi/$(PKG_NAME)/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=3bed8a5051c92cd41ba3477d2db211df8f10fd6e49946f0b74cf643464c1c201

PKG_MAINTAINER:=Carey Sonsino <careys@edgewaterwireless.com>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/dcstad
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Routing and Redirection
  TITLE:=Dual-Channel WiFi client daemon
  URL:=https://www.edgewaterwireless.com
  DEPENDS:=+libdcwsocket +libdcwproto
endef

define Package/dcstad/description
Implementation of the Dual-Channel WiFi client daemon
endef

TARGET_CFLAGS += -ffunction-sections -fdata-sections -flto
TARGET_LDFLAGS += -Wl,--gc-sections

define Package/dcstad/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/dcstad $(1)/bin/
endef

$(eval $(call BuildPackage,dcstad))
