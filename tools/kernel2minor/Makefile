#
# Copyright (C) 2016 adron@yapic.net
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=kernel2minor
PKG_VERSION:=0.24
PKG_RELEASE:=3

PKG_SOURCE_URL:=https://github.com/adron-s/kernel2minor.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=2cacb4dec6673a60a972a3a7f7629464a26a8492
PKG_HASH:=33ca413403a3341af0c9a8e6d9bb58f4ad080a5339e8a8729b83637d35bfaf1b

include $(INCLUDE_DIR)/host-build.mk

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/kernel2minor $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/kernel2minor
endef

$(eval $(call HostBuild))
