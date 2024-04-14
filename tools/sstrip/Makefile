# 
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=sstrip
PKG_VERSION:=3.2

HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/ELFkickers-$(PKG_VERSION)
PKG_SOURCE_URL:=https://www.muppetlabs.com/~breadbox/pub/software
PKG_SOURCE:=ELFkickers-$(PKG_VERSION).tar.gz
PKG_HASH:=9b81e6c53e0c94fc198d9882eb737156f36d565152dc32118897c77b06a2687c

PKG_RELEASE:=1

include $(INCLUDE_DIR)/host-build.mk

define Host/Install
	$(CP) $(HOST_BUILD_DIR)/bin/sstrip $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/sstrip
endef

$(eval $(call HostBuild))
