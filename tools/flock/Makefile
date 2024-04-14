#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME := flock
PKG_VERSION := 2.18
PKG_RELEASE := 1

include $(INCLUDE_DIR)/host-build.mk

HOSTCC := $(HOSTCC_NOCACHE)
HOSTCXX := $(HOSTCXX_NOCACHE)

define Host/Compile
	mkdir -p $(HOST_BUILD_DIR)
	$(HOSTCC) $(HOST_CFLAGS) -o $(HOST_BUILD_DIR)/flock src/flock.c
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/flock $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/flock
endef

$(eval $(call HostBuild))
