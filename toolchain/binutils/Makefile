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

ifeq ($(PKG_VERSION),2.32)
  PKG_HASH:=0ab6c55dd86a92ed561972ba15b9b70a8b9f75557f896446c82e8b36e473ee04
endif

ifeq ($(PKG_VERSION),2.34)
  PKG_HASH:=f00b0e8803dc9bab1e2165bd568528135be734df3fabf8d0161828cd56028952
endif

ifeq ($(PKG_VERSION),2.35.2)
  PKG_HASH:=dcd5b0416e7b0a9b24bed76cd8c6c132526805761863150a26d016415b8bdc7b
endif

ifeq ($(PKG_VERSION),2.36.1)
  PKG_HASH:=e81d9edf373f193af428a0f256674aea62a9d74dfe93f65192d4eae030b0f3b0
endif

ifeq ($(PKG_VERSION),2.37)
  PKG_HASH:=820d9724f020a3e69cb337893a0b63c2db161dadcb0e06fc11dc29eb1e84a32c
endif

HOST_BUILD_PARALLEL:=1

PATCH_DIR:=./patches/$(PKG_VERSION)

include $(INCLUDE_DIR)/toolchain-build.mk

HOST_CONFIGURE_ARGS = \
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
	$(SED) 's, " Linaro.*,,' $(HOST_BUILD_DIR)/bfd/version.h
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
