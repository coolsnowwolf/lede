#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=expat
PKG_VERSION:=2.2.3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_HASH:=b31890fb02f85c002a67491923f89bda5028a880fd6c374f707193ad81aace5f
PKG_SOURCE_URL:=@SF/expat

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

define Host/Install
	$(MAKE) -C $(HOST_BUILD_DIR) install
endef

$(eval $(call HostBuild))
