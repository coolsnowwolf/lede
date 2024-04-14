#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=expat
PKG_CPE_ID:=cpe:/a:libexpat:expat
PKG_VERSION:=2.5.0

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_HASH:=ef2420f0232c087801abf705e89ae65f6257df6b7931d37846a193ef2e8cdcbe
PKG_SOURCE_URL:=https://github.com/libexpat/libexpat/releases/download/R_$(subst .,_,$(PKG_VERSION))

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

HOSTCC := $(HOSTCC_NOCACHE)

HOST_CONFIGURE_ARGS += \
	--disable-shared \
	--without-docbook \
	--with-pic

define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
endef

$(eval $(call HostBuild))
