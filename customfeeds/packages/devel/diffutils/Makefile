#
# Copyright (C) 2008-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=diffutils
PKG_VERSION:=3.7
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/diffutils
PKG_HASH:=b3a7a6221c3dc916085f0d205abf6b8e1ba443d4dd965118da364a1dc1cb3a26

PKG_MAINTAINER:=Roger D <rogerdammit@gmail.com>
PKG_LICENSE:=GPL-3.0
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/diffutils
  SECTION:=devel
  CATEGORY:=Development
  DEPENDS:=+USE_GLIBC:librt
  TITLE:=diffutils
  URL:=http://www.gnu.org/software/diffutils/
  ALTERNATIVES:=\
    200:/usr/bin/cmp:/usr/libexec/cmp-gnu \
    200:/usr/bin/diff:/usr/libexec/diff-gnu
endef

define Package/diffutils/description
  The Diffutils package contains programs that show the differences between
  files or directories.
endef

CONFIGURE_VARS += \
	gl_cv_func_getopt_gnu=yes \
	ac_cv_func_mempcpy=n

define Package/diffutils/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/{sdiff,diff3} $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/libexec
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/diff $(1)/usr/libexec/diff-gnu
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/cmp $(1)/usr/libexec/cmp-gnu
endef

$(eval $(call BuildPackage,diffutils))
