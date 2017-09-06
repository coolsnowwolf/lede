#
# Copyright (C) 2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libelf
PKG_VERSION:=0.8.13
PKG_HASH:=591a9b4ec81c1f2042a97aa60564e0cb79d041c52faa7416acb38bc95bd2c76d
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.mr511.de/software/

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_ARGS += \
	--disable-shared \
	--enable-elf64

define Host/Configure
        (cd $(HOST_BUILD_DIR)/$(3); \
                $(HOST_CONFIGURE_CMD) \
                $(HOST_CONFIGURE_ARGS); \
        )
endef


define Host/Compile
	+$(MAKE) $(HOST_JOBS) -C $(HOST_BUILD_DIR)/lib/ libelf.a
endef

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/{lib,include/libelf}
	$(CP) $(HOST_BUILD_DIR)/lib/{elf_repl.h,gelf.h,libelf.h,nlist.h,sys_elf.h} \
						$(STAGING_DIR_HOST)/include/libelf/
	$(CP) $(HOST_BUILD_DIR)/lib/libelf.a $(STAGING_DIR_HOST)/lib/
endef

define Host/Clean
	rm -rf $(STAGING_DIR_HOST)/include/libelf
	rm -f $(STAGING_DIR_HOST)/lib/libelf.a
	$(call Host/Clean/Default)
endef

$(eval $(call HostBuild))
