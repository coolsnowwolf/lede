# Copyright (c) 2018 Thorsten Glaser <tg@mirbsd.org>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=jupp
PKG_VERSION:=3.1.40
PKG_RELEASE:=1
PKG_LICENSE:=GPL-1.0
PKG_LICENSE_FILES:=COPYING
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_BUILD_PARALLEL:=1
PKG_CONFIG_DEPENDS:=CONFIG_PACKAGE_libncurses

PKG_SOURCE:=joe-$(basename ${PKG_VERSION})jupp$(subst .,,$(suffix ${PKG_VERSION})).tgz
PKG_SOURCE_URL:=http://www.mirbsd.org/MirOS/dist/jupp/ \
		http://pub.allbsd.org/MirOS/dist/jupp/
PKG_HASH:=4bed439cde7f2be294e96e49ef3e913ea90fbe5e914db888403e3a27e8035b1a

include $(INCLUDE_DIR)/package.mk

define Package/jupp/Default
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Editors
  URL:=http://www.mirbsd.org/jupp.htm
  MAINTAINER:=Thorsten Glaser <tg@mirbsd.org>
endef

define Package/jupp
$(call Package/jupp/Default)
  TITLE:=WordStar-inspired text editor
  DEPENDS:=+PACKAGE_libncurses:libncurses
endef

define Package/jupp/description
jupp is a WordStar-inspired text and hex editor,
for programmers and regular end users.
endef

define Package/jupp/chmod_plus_x
	# work around automagic check for +x
	chmod +x ${PKG_BUILD_DIR}/configure
endef
Hooks/Configure/Pre += Package/jupp/chmod_plus_x

CONFIGURE_ARGS+= \
	--disable-selinux \
	--disable-termidx \
	--enable-sysconfjoesubdir=/jupp

ifeq (,${CONFIG_PACKAGE_libncurses})
CONFIGURE_ARGS+= \
	--disable-curses \
	--disable-termcap \
	--disable-terminfo
endif

define Package/jupp/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/joe $(1)/usr/bin/jupp
endef

define Package/jupp-data
$(call Package/jupp/Default)
  TITLE:=jupp Emacs/JOE/Pico emulations, syntax highlighting
  PKGARCH:=all
  DEPENDS:=jupp
  CONFLICTS:=joe
endef

define Package/jupp-data/install
	$(INSTALL_DIR) $(1)/etc/jupp
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/jmacsrc $(1)/etc/jupp/
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/joerc $(1)/etc/jupp/
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/jpicorc $(1)/etc/jupp/
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/jstarrc $(1)/etc/jupp/
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/jupprc $(1)/etc/jupp/
	$(INSTALL_DIR) $(1)/etc/jupp/charmaps
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/charmaps/klingon $(1)/etc/jupp/charmaps/
	$(INSTALL_DIR) $(1)/etc/jupp/syntax
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/syntax/*.jsf $(1)/etc/jupp/syntax/
	$(INSTALL_DIR) $(1)/usr/bin
	$(LN) jupp $(1)/usr/bin/jmacs
	$(LN) jupp $(1)/usr/bin/joe
	$(LN) jupp $(1)/usr/bin/jpico
	$(LN) jupp $(1)/usr/bin/jstar
endef

define Package/jupp-data/conffiles
/etc/jupp
endef

$(eval $(call BuildPackage,jupp))
$(eval $(call BuildPackage,jupp-data))
