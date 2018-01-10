#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gdb-arc
PKG_VERSION:=arc-2016.03-gdb
PKG_RELEASE:=1

PKG_SOURCE:=gdb-arc-2016.03-gdb.tar.gz
PKG_SOURCE_URL:=https://github.com/foss-for-synopsys-dwc-arc-processors/binutils-gdb/archive/$(PKG_VERSION)
PKG_HASH:=6a91f86cc487c1548d3f5d4f29f7226d2019c0db8a63633aeabd5914a340f3f9

PKG_BUILD_DIR:=$(BUILD_DIR)/binutils-gdb-arc-2016.03-gdb

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1
PKG_LICENSE:=GPL-3.0+

include $(INCLUDE_DIR)/package.mk

define Package/gdb-arc/Default
  SECTION:=devel
  CATEGORY:=Development
  DEPENDS:=+!USE_MUSL:libthread-db +PACKAGE_zlib:zlib @arc
  URL:=http://www.gnu.org/software/gdb/
endef

define Package/gdb-arc
$(call Package/gdb-arc/Default)
  TITLE:=GNU Debugger for ARC
  DEPENDS+=+libreadline +libncurses +zlib
endef

define Package/gdb-arc/description
GDB, the GNU Project debugger, allows you to see what is going on `inside'
another program while it executes -- or what another program was doing at the
moment it crashed.
endef

define Package/gdbserver-arc
$(call Package/gdb-arc/Default)
  TITLE:=Remote server for GNU Debugger
endef

define Package/gdbserver-arc/description
GDBSERVER is a program that allows you to run GDB on a different machine than the
one which is running the program being debugged.
endef

# XXX: add --disable-werror to prevent build failure with arm
CONFIGURE_ARGS+= \
	--with-system-readline \
	--without-expat \
	--without-lzma \
	--disable-werror \
	--disable-binutils \
	--disable-ld \
	--disable-gas \
	--disable-sim

CONFIGURE_VARS+= \
	ac_cv_search_tgetent="$(TARGET_LDFLAGS) -lncurses -lreadline"

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		CPPFLAGS="$(TARGET_CPPFLAGS)" \
		all
endef

define Build/Install
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		CPPFLAGS="$(TARGET_CPPFLAGS)" \
		install-gdb
endef

define Package/gdb-arc/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/gdb $(1)/usr/bin/
endef

define Package/gdbserver-arc/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/gdbserver $(1)/usr/bin/
endef

$(eval $(call BuildPackage,gdb-arc))
$(eval $(call BuildPackage,gdbserver-arc))
