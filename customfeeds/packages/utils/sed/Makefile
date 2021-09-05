#
# Copyright (C) 2010-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=sed
PKG_VERSION:=4.8
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/sed
PKG_HASH:=f79b0cfea71b37a8eeec8490db6c5f7ae7719c35587f21edb0617f370eeff633

PKG_MAINTAINER:=Russell Senior <russell@personaltelco.net>
PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:gnu:sed

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/sed
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=sed stream editor utility - full version
  DEPENDS:=+libpcre
  URL:=https://www.gnu.org/software/sed/
  ALTERNATIVES:=300:/bin/sed:/usr/libexec/sed-gnu
endef

define Package/sed/description
sed (stream editor) is a non-interactive command-line text editor. sed is commonly 
used to filter text, i.e., it takes text input, performs some operation (or set of 
operations) on it, and outputs the modified text. sed is typically used for 
extracting part of a file using pattern matching or substituting multiple 
occurrences of a string within a file.
endef

CONFIGURE_ARGS += \
	--disable-acl \
	--without-selinux

define Package/sed/install
	$(INSTALL_DIR) $(1)/usr/libexec
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/sed $(1)/usr/libexec/sed-gnu
endef

$(eval $(call BuildPackage,sed))
