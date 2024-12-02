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
PKG_MIRROR_HASH:=c4487635ea06ea6311a28739c58f7f889b888c91b7ccc0398d225b4bcdeb76a3

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
