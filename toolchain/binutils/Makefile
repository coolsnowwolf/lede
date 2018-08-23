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

TAR_OPTIONS += --exclude='*.rej'

ifeq ($(PKG_VERSION),2.27)
  PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
  PKG_HASH:=369737ce51587f92466041a97ab7d2358c6d9e1b6490b3940eb09fb0a9a6ac88
endif

ifeq ($(PKG_VERSION),2.29.1)
  PKG_HASH:=e7010a46969f9d3e53b650a518663f98a5dde3c3ae21b7d71e5e6803bc36b577
endif

ifeq ($(PKG_VERSION),2.30)
  PKG_HASH:=6e46b8aeae2f727a36f0bd9505e405768a72218f1796f0d09757d45209871ae6
endif

ifneq ($(CONFIG_BINUTILS_VERSION_2_29_ARC),)
  PKG_REV:=arc-2017.09-release
  PKG_SOURCE_URL:=https://github.com/foss-for-synopsys-dwc-arc-processors/binutils-gdb/archive/$(PKG_REV)/
  PKG_SOURCE:=$(PKG_NAME)-$(PKG_REV).tar.gz
  PKG_HASH:=2ea086fd5521e942926dd6ff6922ce31c29ee7ffc754fca3d06385e0f27600f8
  BINUTILS_DIR:=$(PKG_NAME)-gdb-$(PKG_REV)
  HOST_BUILD_DIR:=$(BUILD_DIR_TOOLCHAIN)/$(BINUTILS_DIR)
endif

HOST_BUILD_PARALLEL:=1

PATCH_DIR:=./patches/$(PKG_VERSION)

include $(INCLUDE_DIR)/toolchain-build.mk

BINUTILS_CONFIGURE:= \
	./configure \
		--prefix=$(TOOLCHAIN_DIR) \
		--build=$(GNU_HOST_NAME) \
		--host=$(GNU_HOST_NAME) \
		--target=$(REAL_GNU_TARGET_NAME) \
		--with-sysroot=$(TOOLCHAIN_DIR) \
		--enable-deterministic-archives \
		--enable-plugins \
		--disable-multilib \
		--disable-werror \
		--disable-nls \
		--disable-sim \
		--disable-gdb \
		$(GRAPHITE_CONFIGURE) \
		$(SOFT_FLOAT_CONFIG_OPTION) \
		$(call qstrip,$(CONFIG_EXTRA_BINUTILS_CONFIG_OPTIONS)) 

ifneq ($(CONFIG_SSP_SUPPORT),)
  BINUTILS_CONFIGURE+= \
		--enable-libssp
else
  BINUTILS_CONFIGURE+= \
		--disable-libssp
endif

ifneq ($(CONFIG_EXTRA_TARGET_ARCH),)
  BINUTILS_CONFIGURE+= \
		--enable-targets=$(call qstrip,$(CONFIG_EXTRA_TARGET_ARCH_NAME))-linux-$(TARGET_SUFFIX)
endif

define Host/Prepare
	$(call Host/Prepare/Default)
	ln -snf $(notdir $(HOST_BUILD_DIR)) $(BUILD_DIR_TOOLCHAIN)/$(PKG_NAME)
	$(CP) $(SCRIPT_DIR)/config.{guess,sub} $(HOST_BUILD_DIR)/
	$(SED) 's, " Linaro.*,,' $(HOST_BUILD_DIR)/bfd/version.h
endef

define Host/Configure
	(cd $(HOST_BUILD_DIR); \
		$(BINUTILS_CONFIGURE) \
	);
endef

define Host/Compile
	+$(MAKE) $(HOST_JOBS) -C $(HOST_BUILD_DIR) all
endef

define Host/Install
	mkdir -p $(TOOLCHAIN_DIR)/initial
	$(MAKE) -C $(HOST_BUILD_DIR) \
		prefix=$(TOOLCHAIN_DIR)/initial \
		install
	$(MAKE) -C $(HOST_BUILD_DIR) \
		prefix=$(TOOLCHAIN_DIR) \
		install
	$(call FixupLibdir,$(TOOLCHAIN_DIR)/initial)
	$(RM) $(TOOLCHAIN_DIR)/initial/lib/libiberty.a
	$(CP) $(TOOLCHAIN_DIR)/bin/$(REAL_GNU_TARGET_NAME)-readelf $(HOST_BUILD_PREFIX)/bin/readelf
	# ARC gcc requires extlib.
	# If extlib is not available in "initial" folder
	# initial gcc will fail to build libc.
	if [ -d $(TOOLCHAIN_DIR)/extlib ]; then \
		$(CP) -r $(TOOLCHAIN_DIR)/extlib $(TOOLCHAIN_DIR)/initial/; \
	fi
endef

define Host/Clean
	rm -rf \
		$(HOST_BUILD_DIR) \
		$(BUILD_DIR_TOOLCHAIN)/$(PKG_NAME)
endef

$(eval $(call HostBuild))
