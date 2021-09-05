#
# Copyright (C) 2007-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=file
PKG_VERSION:=5.40
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://download.openpkg.org/components/cache/file/ \
	ftp://ftp.astron.com/pub/file/
PKG_HASH:=167321f43c148a553f68a0ea7f579821ef3b11c27b8cbe158e4df897e4a5dd57

PKG_MAINTAINER:=Marko Ratkaj <marko.ratkaj@sartura.hr>
PKG_LICENSE:=BSD-2-Clause
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/file/Default
  TITLE:=File type determination
  URL:=https://darwinsys.com/file/
endef

define Package/file
$(call Package/file/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE+= utility
  DEPENDS:=+libmagic
endef

define Package/libmagic
$(call Package/file/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE+= library
  DEPENDS:=+zlib +liblzma +libbz2
endef

TARGET_CFLAGS += $(FPIC)
TARGET_LDFLAGS += -Wl,--gc-sections,--as-needed

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \
	--enable-bzlib \
	--enable-xzlib \
	--enable-zlib \
	--disable-libseccomp \
	--disable-rpath \
	--disable-warnings \
	--without-pic

MAKE_PATH := src

define Build/Compile/magic
	( cd $(PKG_BUILD_DIR)/magic/Magdir; \
		for f in `ls`; do \
			cat $$$${f}; \
		done \
	) > $(PKG_BUILD_DIR)/magic/magic
endef
Hooks/Compile/Post += Build/Compile/magic

define Build/Install/magic
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/share/file
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/magic/magic $(PKG_INSTALL_DIR)/usr/share/file/
endef
Hooks/Install/Post += Build/Install/magic

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/magic.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libmagic.{a,so*} $(1)/usr/lib/
endef

define Package/file/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/file $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/share/misc
	$(SED) "/^#/d" $(PKG_INSTALL_DIR)/usr/share/file/magic
	$(SED) "/^$$$$/d" $(PKG_INSTALL_DIR)/usr/share/file/magic
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/file/magic $(1)/usr/share/misc/
endef

define Package/libmagic/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmagic.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,file))
$(eval $(call BuildPackage,libmagic))
