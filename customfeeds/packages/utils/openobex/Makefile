#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=openobex
PKG_VERSION:=1.7.2
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-Source.tar.gz
PKG_SOURCE_URL:=@SF/openobex
PKG_HASH:=158860aaea52f0fce0c8e4b64550daaae06df2689e05834697b7e8c7d73dd4fc
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)-Source

PKG_MAINTAINER:=
PKG_LICENSE:=GPL-2.0-or-later LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:openobex:openobex

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/openobex/Default
  TITLE:=Open Source impl of the OBject EXchange protocol
  URL:=https://www.gitorious.org/openobex
endef

define Package/openobex
$(call Package/openobex/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE+= (meta)
  DEPENDS:=+openobex-apps +libopenobex
endef

define Package/openobex-apps
$(call Package/openobex/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE+= (apps)
  DEPENDS:=+libopenobex +bluez-libs
endef

define Package/libopenobex
$(call Package/openobex/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE+= (library)
  DEPENDS:=+libusb-1.0
endef

CMAKE_OPTIONS += \
	-DBUILD_DOCUMENTATION:BOOL=OFF

define Build/Compile
	$(call Build/Compile/Default,openobex openobex-apps)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/openobex $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libopenobex.so* $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/apps/lib/libopenobex-apps-common.a $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/openobex.pc $(1)/usr/lib/pkgconfig/
endef

define Package/openobex/install
	:
endef

define Package/openobex-apps/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/* $(1)/usr/sbin/
endef

define Package/libopenobex/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libopenobex.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,openobex))
$(eval $(call BuildPackage,openobex-apps))
$(eval $(call BuildPackage,libopenobex))
