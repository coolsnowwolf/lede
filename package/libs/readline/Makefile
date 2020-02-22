#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=readline
PKG_VERSION:=8.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/readline
PKG_HASH:=e339f51971478d369f8a053a330a190781acb9864cf4c541060f12078948e461

PKG_LICENSE:=GPL-3.0
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:gnu:readline

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

define Package/libreadline
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Command lines edition library
  DEPENDS:=+libncursesw
  URL:=http://cnswww.cns.cwru.edu/php/chet/readline/rltop.html
  ABI_VERSION:=8
endef

define Package/libreadline/description
	The Readline library provides a set of functions for use by applications
	that allow users to edit command lines as they are typed in. Both Emacs
	and vi editing modes are available. The Readline library includes
	additional functions to maintain a list of previously-entered command
	lines, to recall and perhaps reedit those lines, and perform csh-like
	history expansion on previous commands.
endef

CONFIGURE_ARGS += --with-curses --disable-install-examples

CONFIGURE_VARS += \
	bash_cv_wcwidth_broken=no \
	bash_cv_func_sigsetjmp=yes \

TARGET_CFLAGS += $(FPIC)

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/readline $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib{history,readline}.{a,so*} $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/readline.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libreadline/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib{history,readline}.so.* $(1)/usr/lib/
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,libreadline))
