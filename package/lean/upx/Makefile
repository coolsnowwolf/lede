#
# Copyright (C) 2011-2020 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=upx
PKG_VERSION:=5.3.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-src.tar.xz
PKG_SOURCE_URL:=https://github.com/upx/upx/releases/download/v$(PKG_VERSION)
PKG_HASH:=6229b5564dd91dd423a5280a7c15b70fc05449d2605bb853ce076743057fcf55
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)-src

PKG_MAINTAINER:=Xingwang Liao <kuoruan@gmail.com>
PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=COPYING LICENSE

HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/$(PKG_NAME)-$(PKG_VERSION)-src

CMAKE_BINARY_SUBDIR:=build/release

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/upx
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libstdcpp
  TITLE:=The Ultimate Packer for eXecutables
  URL:=https://upx.github.io/
endef

define Package/upx/description
UPX is a free, secure, portable, extendable, high-performance
executable packer for several executable formats.
endef

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/bin
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/build/release/upx $(STAGING_DIR_HOST)/bin/upx
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/upx
endef

define Package/upx/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build/release/upx $(1)/usr/bin/upx
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,upx))
