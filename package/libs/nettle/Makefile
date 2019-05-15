#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=nettle
PKG_VERSION:=3.4.1
PKG_RELEASE:=1
PKG_USE_MIPS16:=0

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/nettle
PKG_HASH:=f941cf1535cd5d1819be5ccae5babef01f6db611f9b5a777bae9c7604b8a92ad

PKG_LICENSE:=GPL-2.0+
PKG_LICENSE_FILES:=COPYING
PKG_BUILD_PARALLEL:=0

PKG_CONFIG_DEPENDS := CONFIG_LIBNETTLE_MINI

include $(INCLUDE_DIR)/package.mk

define Package/libnettle
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=GNU crypto library
  URL:=http://www.lysator.liu.se/~nisse/nettle/
  DEPENDS+= +!LIBNETTLE_MINI:libgmp
endef

define Package/libnettle/config
	source "$(SOURCE)/Config.in"
endef

TARGET_CFLAGS += $(FPIC)

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-fat \
	--disable-openssl \
	--disable-documentation \
	--enable-static

ifeq ($(CONFIG_LIBNETTLE_MINI),y)
CONFIGURE_ARGS += --enable-mini-gmp
endif

ifeq ($(CONFIG_CPU_SUBTYPE),neon)
CONFIGURE_ARGS += \
	--enable-arm-neon
endif

define Build/Compile
	$(call Build/Compile/Default, \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		CC="$(TARGET_CC)" \
		libnettle.so libhogweed.so
	+$(MAKE) -i $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		install)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/nettle
	$(CP) $(PKG_INSTALL_DIR)/usr/include/nettle/*.h $(1)/usr/include/nettle/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnettle.{a,so*} $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libhogweed.{a,so*} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/nettle.pc \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/hogweed.pc \
		$(1)/usr/lib/pkgconfig/
endef

define Package/libnettle/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnettle.so.* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libhogweed.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libnettle))
