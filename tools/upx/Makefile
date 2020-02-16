#
# Copyright (C) 2011-2020 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=upx
PKG_VERSION:=3.95

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-src.tar.xz
PKG_SOURCE_URL:=https://github.com/upx/upx/releases/download/v$(PKG_VERSION)
PKG_HASH:=3b0f55468d285c760fcf5ea865a070b27696393002712054c69ff40d8f7f5592

HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/$(PKG_NAME)-$(PKG_VERSION)-src

include $(INCLUDE_DIR)/host-build.mk

UPX_UCLDIR := $(BUILD_DIR_HOST)/ucl-1.03

define Host/Compile
	UPX_UCLDIR=$(UPX_UCLDIR) \
	$(MAKE) -C $(HOST_BUILD_DIR)/src \
		CXXFLAGS_WERROR="" LDFLAGS="$(HOST_LDFLAGS)" \
		CXX="$(HOSTCXX)"
endef

define Host/Install
	$(CP) $(HOST_BUILD_DIR)/src/upx.out $(STAGING_DIR_HOST)/bin/upx
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/upx
endef

$(eval $(call HostBuild))
