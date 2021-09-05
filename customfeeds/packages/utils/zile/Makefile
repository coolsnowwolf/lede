#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=zile
PKG_VERSION:=2.3.24
PKG_RELEASE:=5

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/zile
PKG_HASH:=218bb2af414c6a168101656631a5b2da92f20a965895e1006658cc658b0b7e89
PKG_MAINTAINER:=Ted Hess <thess@kitschensync.net>

PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/zile
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libncursesw
  TITLE:=very small emacs-like editor
  URL:=http://www.gnu.org/software/zile/
  SUBMENU:=Editors
endef

define Package/zile/description
	Zile is a small Emacs clone. Zile is a customizable, self-documenting
	real-time display editor. Zile was written to be as similar as possible
	to Emacs; every Emacs user should feel at home with Zile.
endef

CONFIGURE_VARS += \
	gl_cv_func_getopt_gnu=yes \
	gl_cv_warn__Wmudflap=no \
	gl_cv_warn__fmudflap=no

CONFIGURE_ARGS += \
	--with-ncursesw \


define Package/zile/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/$(PKG_NAME) $(1)/usr/bin/
endef

$(eval $(call BuildPackage,zile))
