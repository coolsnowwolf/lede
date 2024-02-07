#
# Copyright (C) 2011-2020 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=upx
PKG_VERSION:=4.2.2
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/upx/upx.git
PKG_SOURCE_VERSION:=099c3d829e80488af7395a4242b318877e980da4
PKG_MIRROR_HASH:=8bee3cd1ada2e4bfbe396348d2aecb3e044f92cdae2b7010838e36997fe6b3c3

PKG_MAINTAINER:=Xingwang Liao <kuoruan@gmail.com>
PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=COPYING LICENSE

HOST_BUILD_DEPENDS:=ucl/host
MAKE_PATH:=src

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

define Package/upx
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libucl +libstdcpp +zlib
  TITLE:=The Ultimate Packer for eXecutables
  URL:=https://upx.github.io/
endef

define Package/upx/description
UPX is a free, secure, portable, extendable, high-performance
executable packer for several executable formats.
endef

define Host/Compile
	UPX_UCLDIR=$(STAGING_DIR_HOST) \
	$(MAKE) -C $(HOST_BUILD_DIR)/src \
		LDFLAGS="$(HOST_LDFLAGS)" \
		CXX="$(HOSTCXX)"
endef

define Host/Install
	$(CP) $(HOST_BUILD_DIR)/build/release/upx $(STAGING_DIR_HOST)/bin/upx
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
