#
# Copyright (C) 2015 OpenWrt.org
# Copyright (C) 2016 LEDE project
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=make-ext4fs

PKG_SOURCE_URL=$(PROJECT_GIT)/project/make_ext4fs.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2016-08-14
PKG_SOURCE_VERSION:=484903e4332be6c317f531b008cb2a841a18c506
PKG_MIRROR_HASH:=d7ccd5e426b1d15331ff000a37dc15161f6eef594453e970fd584fcde5a25075

include $(INCLUDE_DIR)/host-build.mk

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/make_ext4fs $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/make_ext4fs
endef

$(eval $(call HostBuild))
