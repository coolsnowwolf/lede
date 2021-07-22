# 
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=sstrip
PKG_VERSION:=3.1a

HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/ELFkickers-$(PKG_VERSION)
PKG_SOURCE_URL:=https://www.muppetlabs.com/~breadbox/pub/software
PKG_SOURCE:=ELFkickers-$(PKG_VERSION).tar.gz
PKG_HASH:=06430880aaf4919c5f99fc629da7000347421668c2cf32bced2d401aac276508

PKG_RELEASE:=1

include $(INCLUDE_DIR)/host-build.mk

define Host/Install
	$(CP) $(HOST_BUILD_DIR)/bin/sstrip $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/sstrip
endef

$(eval $(call HostBuild))
