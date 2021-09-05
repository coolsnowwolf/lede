#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libgd
PKG_VERSION:=2.3.2
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://github.com/$(PKG_NAME)/$(PKG_NAME)/releases/download/gd-$(PKG_VERSION)/
PKG_HASH:=478a047084e0d89b83616e4c2cf3c9438175fb0cc55d8c8967f06e0427f7d7fb

PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:libgd:libgd

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libgd/default
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libjpeg +libpng +libwebp +LIBGD_TIFF:libtiff +LIBGD_FREETYPE:libfreetype
  TITLE:=The GD graphics library
  URL:=https://libgd.github.io/
endef

define Package/libgd
  $(call Package/libgd/default)
  MENU:=1
  DEPENDS+=+LIBGD_TIFF:libtiff +LIBGD_FREETYPE:libfreetype
  VARIANT:=default
endef

define Package/libgd-full
  $(call Package/libgd/default)
  DEPENDS+=+libtiff +libfreetype
  TITLE+=(full)
  VARIANT:=full
endef

define Package/libgd/description/default
  GD is an open source code library for the dynamic creation of images by
  programmers. GD creates PNG, JPEG and GIF images, among other formats.
endef

Package/libgd/description=$(Package/libgd/description/default)

define Package/libgd-full/description
  $(call Package/libgd/description/default)
  .
  This variant of the libgd package is compiled will all features enabled.
endef

define Package/libgd/config
	if PACKAGE_libgd
		config LIBGD_TIFF
			bool "TIFF image support"
			default n
			help
				Enable TIFF image support through libtiff
	endif
	if PACKAGE_libgd
		config LIBGD_FREETYPE
			bool "Freetype 2.x library support"
			default n
			help
				Enable Freetype 2.x font engine support through libfreetype
	endif
endef

TARGET_CFLAGS += $(FPIC) -ffunction-sections -fdata-sections -flto

CMAKE_OPTIONS += \
	-DENABLE_FONTCONFIG=OFF \
	-DENABLE_ICONV=OFF \
	-DENABLE_JPEG=ON \
	-DENABLE_LIQ=OFF \
	-DENABLE_PNG=ON \
	-DENABLE_TIFF=$(if $(CONFIG_LIBGD_TIFF),ON,OFF) \
	-DENABLE_WEBP=ON \
	-DENABLE_XPM=OFF \
	-DZLIB_INCLUDE_DIR="$(STAGING_DIR)/usr"

ifeq ($(BUILD_VARIANT),full)
	CMAKE_OPTIONS += \
		-DENABLE_TIFF=ON \
		-DFREETYPE_INCLUDE_DIRS=$(STAGING_DIR)/usr/include/freetype2/ \
		-DENABLE_FREETYPE=ON

else

ifdef CONFIG_LIBGD_FREETYPE
	CMAKE_OPTIONS += \
		-DFREETYPE_INCLUDE_DIRS=$(STAGING_DIR)/usr/include/freetype2/ \
		-DENABLE_FREETYPE=ON
else
	CMAKE_OPTIONS += \
		-DENABLE_FREETYPE=OFF
endif

endif

define Build/InstallDev
	$(call Build/InstallDev/cmake,$(1))
	$(SED) 's,/usr/lib,$(STAGING_DIR)/usr/lib,g' $(1)/usr/lib/pkgconfig/gdlib.pc
	$(SED) 's,/usr/include,$(STAGING_DIR)/include,g' $(1)/usr/lib/pkgconfig/gdlib.pc
endef

define Package/libgd/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgd.so* $(1)/usr/lib/
endef

Package/libgd-full/install=$(Package/libgd/install)

$(eval $(call BuildPackage,libgd))
$(eval $(call BuildPackage,libgd-full))
