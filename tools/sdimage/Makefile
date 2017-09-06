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
PKG_SOURCE_DATE:=2015-09-13
PKG_SOURCE_VERSION:=2b99403b6dc60a22b07eb7a5cc0cb184abb89bdd
PKG_MIRROR_HASH:=b0edd55674cd60268c9b293b47df17db319fd58bd78fdbf26e77039f1a6cf681

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
