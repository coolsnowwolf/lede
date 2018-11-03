#
# Copyright (C) 2016 adron@yapic.net
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=kernel2minor
PKG_VERSION:=0.25
PKG_RELEASE:=1

PKG_SOURCE_URL:=https://github.com/adron-s/kernel2minor.git
PKG_MIRROR_HASH:=6083c46c2fe0da37bacd04d5d5439c0e2a9d00e58ff47a63acfd5057d2aa2145
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=1e5a52c7941945f6d64807ebca4a5923ba5466bd
PKG_HASH:=33ca413403a3341af0c9a8e6d9bb58f4ad080a5339e8a8729b83637d35bfaf1b

include $(INCLUDE_DIR)/host-build.mk

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/kernel2minor $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/kernel2minor
endef

$(eval $(call HostBuild))
