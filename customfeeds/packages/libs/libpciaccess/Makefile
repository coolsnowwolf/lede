# Copyright (C) 2019 Lucian Cristian <lucian.cristian@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=libpciaccess
PKG_VERSION:=0.16
PKG_RELEASE:=1

PKG_SOURCE_URL:=https://www.x.org/releases/individual/lib/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_HASH:=214c9d0d884fdd7375ec8da8dcb91a8d3169f263294c9a90c575bf1938b9f489

PKG_MAINTAINER:= Lucian Cristian <lucian.cristian@gmail.com>
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libpciaccess
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Generic PCI access library
  URL:=https://xorg.freedesktop.org/
endef

CONFIGURE_ARGS += \
	--prefix=/usr

TARGET_LDFLAGS += -Wl,--gc-sections,--as-needed

define Build/InstallDev
	$(INSTALL_DIR) \
	  $(1)/usr/include \
	  $(1)/usr/lib

	$(CP) \
	  $(PKG_INSTALL_DIR)/usr/include/* \
	  $(1)/usr/include/

	$(CP) \
	  $(PKG_INSTALL_DIR)/usr/lib/{pkgconfig,*.so*} \
	  $(1)/usr/lib/
endef

define Package/libpciaccess/install
	$(INSTALL_DIR) \
	  $(1)/usr/lib

	$(CP) \
	  $(PKG_INSTALL_DIR)/usr/lib/*.so* \
	  $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libpciaccess))
