#
# Copyright (C) 2007-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libsamplerate
PKG_VERSION:=0.1.9
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.mega-nerd.com/SRC/
PKG_HASH:=0a7eb168e2f21353fb6d84da152e4512126f7dc48ccb0be80578c565413444c1

PKG_MAINTAINER:=Ted Hess <thess@kitschensync.net>

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libsamplerate
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libsndfile
  TITLE:=Sample Rate Converter
  URL:=http://www.mega-nerd.com/SRC/index.html
endef

define Package/libsamplerate/description
 Secret Rabbit Code (aka libsamplerate) is a Sample Rate
 Converter for audio.
endef

TARGET_CFLAGS += $(FPIC)

CONFIGURE_ARGS += \
	--disable-fftw \

define Build/InstallDev
	$(INSTALL_DIR) $(1)
	$(CP) $(PKG_INSTALL_DIR)/* $(1)/
endef

define Package/libsamplerate/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libsamplerate.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libsamplerate))
