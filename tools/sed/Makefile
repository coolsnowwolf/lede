# 
# Copyright (C) 2006-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=sed
PKG_VERSION:=4.4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/$(PKG_NAME)
PKG_HASH:=cbd6ebc5aaf080ed60d0162d7f6aeae58211a1ee9ba9bb25623daa6cd942683b
export SED:=

HOST_BUILD_PARALLEL:=1

HOSTCC := $(HOSTCC_NOCACHE)
HOSTCXX := $(HOSTCXX_NOCACHE)

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_ARGS += \
	--disable-acl \
	--disable-nls \

HOST_CONFIGURE_VARS += \
	ac_cv_search_setfilecon=no \
	ac_cv_header_selinux_context_h=no \
	ac_cv_header_selinux_selinux_h=no \

define Host/Compile
	+$(MAKE) $(HOST_JOBS) -C $(HOST_BUILD_DIR) SHELL="$(BASH)"
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/sed/sed $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/sed
endef

$(eval $(call HostBuild))
