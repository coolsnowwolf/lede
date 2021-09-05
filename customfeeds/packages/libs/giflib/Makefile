#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=giflib
PKG_VERSION:=5.2.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/giflib
PKG_HASH:=31da5562f44c5f15d63340a09a4fd62b48c45620cd302f77a6d9acf0077879bd

PKG_MAINTAINER:=Ted Hess <thess@kitschensync.net>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:giflib_project:giflib

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/giflib
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=GIF libraries
  URL:=https://sourceforge.net/projects/giflib
endef

define Package/giflib/description
  giflib is a library for reading and writing gif images.
  It is API and ABI compatible with libungif which was in wide use while
  the LZW compression algorithm was patented.
endef

define Package/giflib-utils
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=GIF utilities
  URL:=https://sourceforge.net/projects/giflib
endef

define Package/giflib-utils/description
  These are the utilities that come with giflib.
endef

TARGET_CFLAGS += -ffunction-sections -fdata-sections $(FPIC)
TARGET_LDFLAGS += -Wl,--gc-sections,--as-needed

MAKE_FLAGS += \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	PREFIX=$(CONFIGURE_PREFIX)

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/{lib,include}
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgif.so* $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libgif.a $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include
endef

define Package/giflib/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgif.so* $(1)/usr/lib
endef

define Package/giflib-utils/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
endef

$(eval $(call BuildPackage,giflib))
$(eval $(call BuildPackage,giflib-utils))
