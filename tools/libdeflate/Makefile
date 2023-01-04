#
# Copyright (C) 2022 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=libdeflate
PKG_VERSION:=1.15
PKG_RELEASE:=1

PKG_SOURCE_URL:=https://github.com/ebiggers/libdeflate.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=v$(PKG_VERSION)
PKG_MIRROR_HASH:=122feff4543541b547dc89e832adf262c81911ae1acbccdc591f0353a85b600a

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/programs/libdeflate-gzip $(STAGING_DIR_HOST)/bin/
	$(LN) libdeflate-gzip $(STAGING_DIR_HOST)/bin/libdeflate-gunzip
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/libdeflate-gzip
	rm -f $(STAGING_DIR_HOST)/bin/libdeflate-gunzip
endef

$(eval $(call HostBuild))
