#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gdb
PKG_VERSION:=11.2
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/gdb
PKG_HASH:=1497c36a71881b8671a9a84a0ee40faab788ca30d7ba19d8463c3cc787152e32

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1
PKG_LICENSE:=GPL-3.0+
PKG_CPE_ID:=cpe:/a:gnu:gdb

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/gdb/Default
  SECTION:=devel
  CATEGORY:=Development
  DEPENDS:=+!USE_MUSL:libthread-db $(ICONV_DEPENDS) $(INTL_DEPENDS)
  URL:=https://www.gnu.org/software/gdb/
endef

define Package/gdb
$(call Package/gdb/Default)
  TITLE:=GNU Debugger
  DEPENDS+=+libreadline +libncurses +zlib +libgmp
endef

define Package/gdb/description
GDB, the GNU Project debugger, allows you to see what is going on `inside'
another program while it executes -- or what another program was doing at the
moment it crashed.
endef

define Package/gdbserver
$(call Package/gdb/Default)
  TITLE:=Remote server for GNU Debugger
endef

define Package/gdbserver/description
GDBSERVER is a program that allows you to run GDB on a different machine than the
one which is running the program being debugged.
endef

# XXX: add --disable-werror to prevent build failure with arm
CONFIGURE_ARGS+= \
	--with-system-readline \
	--with-system-zlib \
	--without-expat \
	--without-lzma \
	--disable-unit-tests \
	--disable-ubsan \
	--disable-sim \
	--disable-werror \
	--disable-source-highlight \
	--without-mpc \
	--without-mpfr \
	--without-isl \
	--with-libgmp-prefix=$(STAGING_DIR)/usr

CONFIGURE_VARS+= \
	ac_cv_search_tgetent="$(TARGET_LDFLAGS) -lncurses -lreadline"

TARGET_LDFLAGS+= \
	$(INTL_LDFLAGS) $(if $(INTL_FULL),-lintl) \
	-static-libstdc++ \
	-Wl,--gc-sections

define Build/Install
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		CPPFLAGS="$(TARGET_CPPFLAGS)" \
		install-gdb install-gdbserver
endef

define Package/gdb/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/gdb $(1)/usr/bin/
endef

define Package/gdbserver/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/gdbserver $(1)/usr/bin/
endef

$(eval $(call BuildPackage,gdb))
$(eval $(call BuildPackage,gdbserver))
