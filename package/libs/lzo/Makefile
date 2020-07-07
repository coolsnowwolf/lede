#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lzo
PKG_VERSION:=2.10
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.oberhumer.com/opensource/lzo/download/
PKG_HASH:=c0f892943208266f9b6543b3ae308fab6284c5c90e627931446fb49b4221a072

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_LICENSE:=GPL-2.0+

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/liblzo
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=A real-time data compression library
  URL:=http://www.oberhumer.com/opensource/lzo/
  ABI_VERSION:=2
endef

define Package/liblzo/description
 LZO is a data compression library which is suitable for data de-/compression
 in real-time. This means it favours speed over compression ratio.
endef

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \

TARGET_CFLAGS += $(FPIC)
MAKE_FLAGS += CFLAGS_O="$(TARGET_CFLAGS)"

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/include/lzo $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/liblzo2.{a,so*} $(1)/usr/lib/
endef

define Package/liblzo/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/liblzo2.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,liblzo))
