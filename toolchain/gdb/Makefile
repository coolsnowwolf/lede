#
# Copyright (C) 2006-2020 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=gdb
PKG_VERSION:=14.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/gdb
PKG_HASH:=d66df51276143451fcbff464cc8723d68f1e9df45a6a2d5635a54e71643edb80
PKG_CPE_ID:=cpe:/a:gnu:gdb
GDB_DIR:=$(PKG_NAME)-$(PKG_VERSION)

HOST_BUILD_DIR:=$(BUILD_DIR_TOOLCHAIN)/$(GDB_DIR)

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/toolchain-build.mk

export ZSTD_CFLAGS=-I$(STAGING_DIR_HOST)/include -pthread
export ZSTD_LIBS=-L$(STAGING_DIR_HOST)/lib -lzstd -lpthread

HOST_CONFIGURE_VARS += \
	acx_cv_cc_gcc_supports_ada=false \
	gdb_cv_func_sigsetjmp=yes

HOST_CONFIGURE_ARGS = \
	--prefix=$(TOOLCHAIN_DIR) \
	--build=$(GNU_HOST_NAME) \
	--host=$(GNU_HOST_NAME) \
	--target=$(REAL_GNU_TARGET_NAME) \
	--with-gmp=$(STAGING_DIR_HOST) \
	--with-mpfr=$(STAGING_DIR_HOST) \
	--with-mpc=$(STAGING_DIR_HOST) \
	--with-expat=$(STAGING_DIR_HOST) \
	--disable-werror \
	--without-uiout \
	--enable-tui --disable-gdbtk --without-x \
	--without-included-gettext \
	--enable-threads \
	--disable-unit-tests \
	--disable-ubsan \
	--disable-binutils \
	--disable-ld \
	--disable-gas \
	--disable-sim

ifneq ($(CONFIG_GDB_PYTHON),)
  HOST_CONFIGURE_ARGS+= --with-python
else
  HOST_CONFIGURE_ARGS+= --without-python
endif

define Host/Install
	mkdir -p $(TOOLCHAIN_DIR)/bin
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/gdb/gdb $(TOOLCHAIN_DIR)/bin/$(TARGET_CROSS)gdb
	ln -fs $(TARGET_CROSS)gdb $(TOOLCHAIN_DIR)/bin/$(GNU_TARGET_NAME)-gdb
	strip $(TOOLCHAIN_DIR)/bin/$(TARGET_CROSS)gdb
	mkdir -p $(TOOLCHAIN_DIR)/share/gdb
	-cp -R $(HOST_BUILD_DIR)/gdb/data-directory/python $(TOOLCHAIN_DIR)/share/gdb/
	cp -R $(HOST_BUILD_DIR)/gdb/data-directory/syscalls $(TOOLCHAIN_DIR)/share/gdb/
	cp -R $(HOST_BUILD_DIR)/gdb/data-directory/system-gdbinit $(TOOLCHAIN_DIR)/share/gdb/
endef

define Host/Clean
	rm -rf \
		$(HOST_BUILD_DIR) \
		$(TOOLCHAIN_DIR)/bin/$(TARGET_CROSS)gdb \
		$(TOOLCHAIN_DIR)/bin/$(GNU_TARGET_NAME)-gdb
endef

$(eval $(call HostBuild))
