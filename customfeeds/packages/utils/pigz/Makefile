include $(TOPDIR)/rules.mk

PKG_NAME:=pigz
PKG_VERSION:=2.4
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://zlib.net/pigz/
PKG_HASH:=a4f816222a7b4269bd232680590b579ccc72591f1bb5adafcd7208ca77e14f73

PKG_MAINTAINER:=Ken Wong <xinxijishuwyq@gmail.com>
PKG_LICENSE:=CUSTOM
PKG_LICENSE_FILES:=README
PKG_CPE_ID:=cpe:/a:zlib:pigz

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/pigz
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Compression
  TITLE:=pigz
  URL:=https://zlib.net/pigz
  DEPENDS:=+zlib
endef

define Package/pigz/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/pigz $(1)/usr/bin/pigz
endef

$(eval $(call BuildPackage,pigz))

