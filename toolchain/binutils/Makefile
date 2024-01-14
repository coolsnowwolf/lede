#
# Copyright (C) 2006-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=binutils
PKG_VERSION:=$(call qstrip,$(CONFIG_BINUTILS_VERSION))
BIN_VERSION:=$(PKG_VERSION)

PKG_SOURCE_URL:=@GNU/binutils/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_CPE_ID:=cpe:/a:gnu:binutils

TAR_OPTIONS += --exclude='*.rej'

ifeq ($(PKG_VERSION),2.37)
  PKG_HASH:=820d9724f020a3e69cb337893a0b63c2db161dadcb0e06fc11dc29eb1e84a32c
endif

ifeq ($(PKG_VERSION),2.38)
  PKG_HASH:=e316477a914f567eccc34d5d29785b8b0f5a10208d36bbacedcc39048ecfe024
endif

ifeq ($(PKG_VERSION),2.39)
  PKG_HASH:=645c25f563b8adc0a81dbd6a41cffbf4d37083a382e02d5d3df4f65c09516d00
endif

ifeq ($(PKG_VERSION),2.40)
  PKG_HASH:=0f8a4c272d7f17f369ded10a4aca28b8e304828e95526da482b0ccc4dfc9d8e1
endif

ifeq ($(PKG_VERSION),2.41)
  PKG_HASH:=ae9a5789e23459e59606e6714723f2d3ffc31c03174191ef0d015bdf06007450
endif

HOST_BUILD_PARALLEL:=1

PATCH_DIR:=./patches/$(PKG_VERSION)

include $(INCLUDE_DIR)/toolchain-build.mk

ifdef CONFIG_GCC_USE_GRAPHITE
  GRAPHITE_CONFIGURE:= --with-isl=$(STAGING_DIR_HOST)
else
  GRAPHITE_CONFIGURE:= --without-isl --without-cloog
endif

HOST_CONFIGURE_ARGS = \
	--prefix=$(TOOLCHAIN_DIR) \
	--build=$(GNU_HOST_NAME) \
	--host=$(GNU_HOST_NAME) \
	--target=$(REAL_GNU_TARGET_NAME) \
	--with-sysroot=$(TOOLCHAIN_DIR) \
	--with-system-zlib \
	--without-zstd \
	--enable-deterministic-archives \
	--enable-plugins \
	--enable-lto \
	--disable-gprofng \
	--disable-multilib \
	--disable-werror \
	--disable-nls \
	--disable-sim \
	--disable-gdb \
	$(GRAPHITE_CONFIGURE) \
	$(SOFT_FLOAT_CONFIG_OPTION) \
	$(call qstrip,$(CONFIG_EXTRA_BINUTILS_CONFIG_OPTIONS))

ifneq ($(CONFIG_SSP_SUPPORT),)
  HOST_CONFIGURE_ARGS+= \
		--enable-libssp
else
  HOST_CONFIGURE_ARGS+= \
		--disable-libssp
endif

ifneq ($(CONFIG_EXTRA_TARGET_ARCH),)
  HOST_CONFIGURE_ARGS+= \
		--enable-targets=$(call qstrip,$(CONFIG_EXTRA_TARGET_ARCH_NAME))-linux-$(TARGET_SUFFIX)
endif

HOST_CONFIGURE_VARS += \
	acx_cv_cc_gcc_supports_ada=false

define Host/Prepare
	$(call Host/Prepare/Default)
	ln -snf $(notdir $(HOST_BUILD_DIR)) $(BUILD_DIR_TOOLCHAIN)/$(PKG_NAME)
	$(CP) $(SCRIPT_DIR)/config.{guess,sub} $(HOST_BUILD_DIR)/
endef

define Host/Compile
	+$(MAKE) $(HOST_JOBS) -C $(HOST_BUILD_DIR) all
endef

define Host/Install
	$(MAKE) -C $(HOST_BUILD_DIR) \
		install
	$(call FixupLibdir,$(TOOLCHAIN_DIR))
	$(CP) $(TOOLCHAIN_DIR)/bin/$(REAL_GNU_TARGET_NAME)-readelf $(HOST_BUILD_PREFIX)/bin/readelf
endef

define Host/Clean
	rm -rf \
		$(HOST_BUILD_DIR) \
		$(BUILD_DIR_TOOLCHAIN)/$(PKG_NAME)
endef

$(eval $(call HostBuild))
