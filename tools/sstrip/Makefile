# 
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=sstrip

include $(INCLUDE_DIR)/host-build.mk

define Host/Compile
	$(HOSTCC) $(HOST_CFLAGS) -include endian.h -o $(HOST_BUILD_DIR)/sstrip src/sstrip.c
endef

define Host/Install
	$(CP) $(HOST_BUILD_DIR)/sstrip $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/sstrip
endef

$(eval $(call HostBuild))
