#
# Copyright (C) 2006-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=unzip
PKG_REV:=60
PKG_VERSION:=6.0
PKG_RELEASE:=8

PKG_SOURCE:=$(PKG_NAME)$(PKG_REV).tar.gz
PKG_SOURCE_URL:=@SF/infozip
PKG_HASH:=036d96991646d0449ed0aa952e4fbe21b476ce994abc276e49d30e686708bd37
PKG_MAINTAINER:=Álvaro Fernández Rojas <noltari@gmail.com>

PKG_LICENSE:=BSD-4-Clause
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:info-zip:unzip

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)/unzip$(PKG_REV)
PKG_CHECK_FORMAT_SECURITY:=0

include $(INCLUDE_DIR)/package.mk

define Package/unzip
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=
  TITLE:=De-archiver for .zip files
  URL:=http://infozip.sourceforge.net/UnZip.html
  SUBMENU:=Compression
endef

define Package/unzip/description
	InfoZIP's unzip program. With the exception of multi-volume archives
	(ie, .ZIP files that are split across several disks using PKZIP's /& option),
	this can handle any file produced either by PKZIP, or the corresponding
	InfoZIP zip program.
endef

define Build/Configure
endef

TARGET_CFLAGS += \
	-DNO_LCHMOD \
	-DLARGE_FILE_SUPPORT \
	-DUNICODE_WCHAR \
	-DUNICODE_SUPPORT \
	-DUTF8_MAYBE_NATIVE \
	-DZIP64_SUPPORT

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) -f unix/Makefile unzips \
		$(TARGET_CONFIGURE_OPTS) \
		prefix="$(PKG_INSTALL_DIR)/usr" \
		CFLAGS="$(TARGET_CFLAGS)" \
		CF="$(TARGET_CFLAGS) -O $(TARGET_CPPFLAGS) -I. -DUNIX" \
		CC="$(TARGET_CC)" \
		LD="$(TARGET_CC) $(TARGET_LDFLAGS)" \
		install
endef

define Package/unzip/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/{funzip,unzip,unzipsfx,zipgrep} \
		$(1)/usr/bin/
	$(LN) unzip $(1)/usr/bin/zipinfo
endef

$(eval $(call BuildPackage,unzip))
