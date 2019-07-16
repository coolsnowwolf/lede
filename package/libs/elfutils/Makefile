#
# Copyright (C) 2010-2014 OpenWrt.org
# Copyright (C) 2016-2017 Luiz Angelo Daros de Luca <luizluca@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=elfutils
PKG_VERSION:=0.175
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://sourceware.org/$(PKG_NAME)/ftp/$(PKG_VERSION)
PKG_HASH:=f7ef925541ee32c6d15ae5cb27da5f119e01a5ccdbe9fe57bf836730d7b7a65b
PKG_MAINTAINER:=Luiz Angelo Daros de Luca <luizluca@gmail.com>
PKG_LICENSE:=GPL-3.0+
PKG_LICENSE_FILES:=COPYING COPYING-GPLV2 COPYING-LGPLV3
PKG_CPE_ID:=cpe:/a:elfutils_project:elfutils
PKG_FIXUP:=autoreconf

PKG_INSTALL:=1
PKG_USE_MIPS16:=0

PKG_BUILD_DEPENDS:=USE_UCLIBC:argp-standalone USE_MUSL:argp-standalone

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/elfutils/Default
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=ELF manipulation libraries
  URL:=https://fedorahosted.org/elfutils/
  ABI_VERSION:=1
endef

define Package/libasm
  $(call Package/elfutils/Default)
  TITLE+= (libasm)
  DEPENDS:=+libelf +libdw
endef

define Package/libdw
  $(call Package/elfutils/Default)
  DEPENDS:=+libelf +libbz2
  TITLE+= (libdw)
endef

define Package/libelf
  $(call Package/elfutils/Default)
  DEPENDS:=$(INTL_DEPENDS) +zlib
  TITLE+= (libelf)
  PROVIDES:=libelf1
endef

ifeq ($(CONFIG_BUILD_NLS),y)
TARGET_LDFLAGS += -lintl
else
CONFIGURE_ARGS += --disable-nls
endif

CONFIGURE_ARGS += \
	--program-prefix=eu- \
	--without-lzma

TARGET_CFLAGS += -D_GNU_SOURCE -Wno-unused-result -Wno-format-nonliteral

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libasm*.{a,so*} $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libdw*.{a,so*} $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelf*.{a,so*} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libelf.pc $(1)/usr/lib/pkgconfig/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libdw.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libasm/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libasm{-*.so,*.so.*} $(1)/usr/lib/
endef

define Package/libdw/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libdw{-*.so,*.so.*} $(1)/usr/lib/
endef

define Package/libelf/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelf{-*.so,*.so.*} $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libasm))
$(eval $(call BuildPackage,libdw))
$(eval $(call BuildPackage,libelf))
