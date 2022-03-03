#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/target.mk

PKG_NAME:=fortify-headers
PKG_VERSION:=1.1
PKG_RELEASE=1

PKG_SOURCE_URL:=http://dl.2f30.org/releases
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=6ba5d860a2d2ba4c3346924b93930c34856eafe148bdbdf271ecab8065201fb6

include $(INCLUDE_DIR)/toolchain-build.mk

define Host/Compile
	true
endef

define Host/Install
	$(MAKE) -C $(HOST_BUILD_DIR) PREFIX="" DESTDIR="$(TOOLCHAIN_DIR)" install
endef

$(eval $(call HostBuild))
