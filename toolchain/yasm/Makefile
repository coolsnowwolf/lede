#
# Copyright (C) 2016 Daniel Golle <daniel@makrotopia.org>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=yasm
PKG_VERSION:=1.3.0

PKG_SOURCE_URL:=http://www.tortall.net/projects/yasm/releases/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

PKG_HASH:=3dce6601b495f5b3d45b59f7d2492a340ee7e84b5beca17e48f862502bd5603f

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/toolchain-build.mk

YASM_CONFIGURE:= \
	./configure \
		--prefix=$(TOOLCHAIN_DIR) \
		--build=$(GNU_HOST_NAME) \
		--host=$(GNU_HOST_NAME) \
		--target=$(REAL_GNU_TARGET_NAME) \
		--with-sysroot=$(TOOLCHAIN_DIR) \
		--disable-multilib \
		--disable-werror \
		--disable-nls \
		--disable-sim \
		--disable-gdb \
		$(SOFT_FLOAT_CONFIG_OPTION) \

define Host/Prepare
	$(call Host/Prepare/Default)
	ln -snf $(notdir $(HOST_BUILD_DIR)) $(BUILD_DIR_TOOLCHAIN)/$(PKG_NAME)
	$(CP) $(SCRIPT_DIR)/config.{guess,sub} $(HOST_BUILD_DIR)/
endef

define Host/Configure
	(cd $(HOST_BUILD_DIR); \
		$(YASM_CONFIGURE) \
	);
endef

define Host/Compile
	+$(MAKE) $(HOST_JOBS) -C $(HOST_BUILD_DIR) all
endef

define Host/Install
	$(MAKE) -C $(HOST_BUILD_DIR) \
		prefix=$(TOOLCHAIN_DIR) \
		install
endef

define Host/Clean
	rm -rf \
		$(HOST_BUILD_DIR) \
		$(BUILD_DIR_TOOLCHAIN)/$(PKG_NAME)
endef

$(eval $(call HostBuild))
