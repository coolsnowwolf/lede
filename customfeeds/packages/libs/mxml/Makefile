include $(TOPDIR)/rules.mk

PKG_NAME:=mxml
PKG_VERSION:=3.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/michaelrsweet/$(PKG_NAME)/releases/download/v$(PKG_VERSION)/
PKG_HASH:=b894f6c64964f2e77902564c17ba00f5d077a7a24054e7c1937903b0bd42c974
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

PKG_FIXUP:=autoreconf

PKG_MAINTAINER:=Espen Jürgensen <espenjurgensen+openwrt@gmail.com>
PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk

define Package/mxml
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Mini-XML
  URL:=http://www.minixml.org/
  DEPENDS:=+zlib
endef

define Package/mxml/description
  A small xml library.
endef

TARGET_CFLAGS += $(FPIC)

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static

define Build/InstallDev
	mkdir -p $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/mxml.h $(1)/usr/include/
	mkdir -p $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libmxml.so* $(1)/usr/lib/
	mkdir -p $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_BUILD_DIR)/mxml.pc $(1)/usr/lib/pkgconfig/
endef

define Package/mxml/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libmxml.so.*  $(1)/usr/lib/
endef

$(eval $(call BuildPackage,mxml))
