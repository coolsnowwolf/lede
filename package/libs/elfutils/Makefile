#
# Copyright (C) 2010-2014 OpenWrt.org
# Copyright (C) 2016-2017 Luiz Angelo Daros de Luca <luizluca@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=elfutils
PKG_VERSION:=0.169
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://sourceware.org/$(PKG_NAME)/ftp/$(PKG_VERSION)
PKG_HASH:=9412fac7b30872b738bc1ed1ebcaed54493c26ef9a67887913498c17b10f3bc2
PKG_MAINTAINER:=Luiz Angelo Daros de Luca <luizluca@gmail.com>
PKG_LICENSE:=GPL-3.0+
PKG_LICENSE_FILES:=COPYING COPYING-GPLV2 COPYING-LGPLV3
PKG_CPE_ID:=cpe:/a:elfutils_project:elfutils

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
endef

define Package/libasm
  $(call Package/elfutils/Default)
  TITLE+= (libasm)
  DEPENDS:=libelf1 +libdw
endef

define Package/libdw
  $(call Package/elfutils/Default)
  DEPENDS:=libelf1 +libbz2
  TITLE+= (libdw)
endef

define Package/libelf1
  $(call Package/elfutils/Default)
  DEPENDS:=$(INTL_DEPENDS) +zlib
  TITLE+= (libelf)
endef

ifeq ($(CONFIG_BUILD_NLS),y)
TARGET_LDFLAGS += "-lintl"
endif

ifdef CONFIG_USE_UCLIBC
CONFIGURE_VARS += \
	LIBS="-largp"
endif

ifdef CONFIG_USE_MUSL
CONFIGURE_VARS += \
	LIBS="-largp"
endif

CONFIGURE_ARGS += \
	--without-lzma

TARGET_CFLAGS += -D_GNU_SOURCE -Wno-unused-result -Wno-format-nonliteral

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libasm/libasm.{a,so*} $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/libdw/libdw.{a,so*} $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/libelf/libelf.{a,so*} $(1)/usr/lib/
endef

define Package/libasm/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libasm/libasm.so* $(1)/usr/lib/
endef

define Package/libdw/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libdw/libdw.so* $(1)/usr/lib/
endef

define Package/libelf1/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libelf/libelf.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libasm))
$(eval $(call BuildPackage,libdw))
$(eval $(call BuildPackage,libelf1))
