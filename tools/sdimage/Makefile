#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=imx-uuc
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/mhei/fsl-imx-uuc.git
PKG_SOURCE_DATE:=2016-01-10
PKG_SOURCE_VERSION:=d395b310b6c0f41ab1302242a639e7d0d7767207
PKG_MIRROR_HASH:=d67ec9d509201511bc7cba9f7551baa729361817f97390f7c5cbb99b5a7b89b1

PKG_LICENSE:=GPL-2.0+
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/host-build.mk

define Host/Configure
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/sdimage $(STAGING_DIR_HOST)/bin/sdimage
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/sdimage
endef

$(eval $(call HostBuild))
