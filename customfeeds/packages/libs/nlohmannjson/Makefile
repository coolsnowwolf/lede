#
# This is free software, licensed under the GNU General Public License v2.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=nlohmannjson
PKG_VERSION:=3.9.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).zip
PKG_SOURCE_URL:=https://codeload.github.com/nlohmann/json/zip/v$(PKG_VERSION)?
PKG_HASH:=a88449d68aab8d027c5beefe911ba217f5ffcc0686ae1793d37f3d20698b37c6
PKG_BUILD_DIR:=$(BUILD_DIR)/json-$(PKG_VERSION)

PKG_MAINTAINER:=Leonid Esman <leonid.esman@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE.MIT

PKG_INSTALL:=1
PKG_BUILD_DEPENDS:=meson/host

include $(INCLUDE_DIR)/package.mk
include ../../devel/meson/meson.mk

define Package/nlohmannjson
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=JSON for Modern C++
  URL:=https://nlohmann.github.io/json/
  BUILDONLY:=1
endef

define Package/nlohmannjson/description
  Niels Lohmann's JSON headers-only library for modern C++
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/nlohmann
	$(CP) $(PKG_INSTALL_DIR)/usr/include/nlohmann/json.hpp $(1)/usr/include/nlohmann
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/nlohmann_json.pc $(1)/usr/lib/pkgconfig
endef

$(eval $(call BuildPackage,nlohmannjson))
