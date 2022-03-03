#
# Copyright (C) 2006-2020 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=gdb
PKG_VERSION:=11.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/gdb
PKG_HASH:=1497c36a71881b8671a9a84a0ee40faab788ca30d7ba19d8463c3cc787152e32
GDB_DIR:=$(PKG_NAME)-$(PKG_VERSION)

HOST_BUILD_DIR:=$(BUILD_DIR_TOOLCHAIN)/$(GDB_DIR)

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/toolchain-build.mk

HOST_CONFIGURE_VARS += \
	acx_cv_cc_gcc_supports_ada=false \
	gdb_cv_func_sigsetjmp=yes

HOST_CONFIGURE_ARGS = \
	--prefix=$(TOOLCHAIN_DIR) \
	--build=$(GNU_HOST_NAME) \
	--host=$(GNU_HOST_NAME) \
	--target=$(REAL_GNU_TARGET_NAME) \
	--with-gmp=$(TOPDIR)/staging_dir/host \
	--with-mpfr=$(TOPDIR)/staging_dir/host \
	--with-mpc=$(TOPDIR)/staging_dir/host \
	--disable-werror \
	--without-uiout \
	--enable-tui --disable-gdbtk --without-x \
	--without-included-gettext \
	--enable-threads \
	--with-expat \
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
