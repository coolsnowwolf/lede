#
# Copyright (C) 2019 EWSI
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dcwapd
PKG_VERSION:=1.1.0
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/ewsi/$(PKG_NAME)/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=58e52bf4e7526b2f26319740549dbcc6f6ab505f587815ee8731e40f7fecb625

PKG_MAINTAINER:=Carey Sonsino <careys@edgewaterwireless.com>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/dcwapd
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Routing and Redirection
  TITLE:=Dual-Channel WiFi AP daemon
  URL:=https://www.edgewaterwireless.com
  DEPENDS:=+libstdcpp +kmod-macremapper +libdcwsocket +libdcwproto +mrmctl +libuci
endef

define Package/dcwapd/description
Implementation of the Dual-Channel WiFi AP daemon
endef

CONFIGURE_ARGS += \
	--enable-platform=linuxjsonstatic \
	--enable-shared

TARGET_CXXFLAGS += -std=c++11 -DRAPIDJSON_HAS_CXX11_RVALUE_REFS=0 -ffunction-sections -fdata-sections -flto
TARGET_LDFLAGS += -ldcwproto -ldcwsocket -lmrmfilterparser -luci -Wl,--gc-sections,--as-needed

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
endef

define Package/dcwapd/conffiles
/etc/config/dcwapd
endef

define Package/dcwapd/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/$(PKG_NAME) $(1)/bin/
	$(INSTALL_DIR) $(1)/usr/lib
	# Note: $(INSTALL_BIN) does not keep symlinks, so use $(CP)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/

#	UCI config file
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/dcwapd.uci $(1)/etc/config/dcwapd

#	Init script
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/dcwapd.init $(1)/etc/init.d/dcwapd
endef
$(eval $(call BuildPackage,dcwapd))
