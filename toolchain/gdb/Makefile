#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=gdb

ifeq ($(CONFIG_arc),y)
PKG_VERSION:=arc-2017.03-gdb

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/foss-for-synopsys-dwc-arc-processors/binutils-gdb/archive/arc-2017.03-gdb
PKG_HASH:=f47930161e9e99c46ab018dabbf56024739dda29c51f92495eaa0d4d7e3511f6
GDB_DIR:=binutils-$(PKG_NAME)-$(PKG_VERSION)
PATCH_DIR:=./patches-arc
else
PKG_VERSION:=8.0.1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/gdb
PKG_HASH:=3dbd5f93e36ba2815ad0efab030dcd0c7b211d7b353a40a53f4c02d7d56295e3
GDB_DIR:=$(PKG_NAME)-$(PKG_VERSION)
endif

HOST_BUILD_DIR:=$(BUILD_DIR_TOOLCHAIN)/$(GDB_DIR)

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/toolchain-build.mk

HOST_CONFIGURE_VARS += \
	gdb_cv_func_sigsetjmp=yes

HOST_CONFIGURE_ARGS = \
	--prefix=$(TOOLCHAIN_DIR) \
	--build=$(GNU_HOST_NAME) \
	--host=$(GNU_HOST_NAME) \
	--target=$(REAL_GNU_TARGET_NAME) \
	--disable-werror \
	--without-uiout \
	--disable-tui --disable-gdbtk --without-x \
	--without-included-gettext \
	--enable-threads \
	--with-expat \
	--without-python \
	--disable-binutils \
	--disable-ld \
	--disable-gas \
	--disable-sim

define Host/Install
	mkdir -p $(TOOLCHAIN_DIR)/bin
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/gdb/gdb $(TOOLCHAIN_DIR)/bin/$(TARGET_CROSS)gdb
	ln -fs $(TARGET_CROSS)gdb $(TOOLCHAIN_DIR)/bin/$(GNU_TARGET_NAME)-gdb
	strip $(TOOLCHAIN_DIR)/bin/$(TARGET_CROSS)gdb
endef

define Host/Clean
	rm -rf \
		$(HOST_BUILD_DIR) \
		$(TOOLCHAIN_DIR)/bin/$(TARGET_CROSS)gdb \
		$(TOOLCHAIN_DIR)/bin/$(GNU_TARGET_NAME)-gdb
endef

$(eval $(call HostBuild))
