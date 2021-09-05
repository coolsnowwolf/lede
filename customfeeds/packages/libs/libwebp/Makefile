include $(TOPDIR)/rules.mk

PKG_NAME:=libwebp
PKG_VERSION:=1.2.0
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://storage.googleapis.com/downloads.webmproject.org/releases/webp
PKG_HASH:=2fc8bbde9f97f2ab403c0224fb9ca62b2e6852cbc519e91ceaa7c153ffd88a0c

PKG_MAINTAINER:=
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=COPYING

CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libwebp
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=WebP library
  URL:=https://www.webmproject.org
endef

define Package/libwebp/description
  The libwebp package contains a library for the WebP format.
endef

CMAKE_OPTIONS += \
	-DBUILD_SHARED_LIBS=ON \
	-DWEBP_BUILD_ANIM_UTILS=OFF \
	-DWEBP_BUILD_CWEBP=OFF \
	-DWEBP_BUILD_DWEBP=OFF \
	-DWEBP_BUILD_GIF2WEBP=OFF \
	-DWEBP_BUILD_IMG2WEBP=OFF \
	-DWEBP_BUILD_VWEBP=OFF \
	-DWEBP_BUILD_WEBPINFO=OFF \
	-DWEBP_BUILD_WEBPMUX=OFF \
	-DWEBP_BUILD_EXTRAS=OFF

ifneq ($(findstring arm,$(CONFIG_ARCH)),)
ifeq ($(findstring neon,$(CONFIG_CPU_TYPE)),)
CMAKE_OPTIONS += -DWEBP_ENABLE_SIMD=OFF
endif
endif

TARGET_CFLAGS += -flto

define Package/libwebp/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libwebp*.s* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libwebp))
