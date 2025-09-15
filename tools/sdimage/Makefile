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
PKG_SOURCE_DATE:=2018-11-18
PKG_SOURCE_VERSION:=c6536ac5b4388b33c217bde2c3a76a4e96d64176
PKG_MIRROR_HASH:=747fb640a0596cab7d516979188e1f85be58c470df85cf1f7e3dcf8a4c1b36e5

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
