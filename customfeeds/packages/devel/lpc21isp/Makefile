#
# Copyright (C) 2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lpc21isp
PKG_VERSION:=197
PKG_RELEASE:=5
PKG_LICENSE:=LGPL-3.0-or-later
PKG_LICENSE_FILES:=README gpl.txt lgpl-3.0.txt

PKG_BUILD_DIR:=$(BUILD_DIR)/lpc21isp_$(PKG_VERSION)
PKG_SOURCE:=lpc21isp_$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/lpc21isp
PKG_HASH:=9f7d80382e4b70bfa4200233466f29f73a36fea7dc604e32f05b9aa69ef591dc
PKG_CAT:=zcat

include $(INCLUDE_DIR)/package.mk

define Package/lpc21isp
  SECTION:=base
  CATEGORY:=Development
  TITLE:=Command line ISP for NXP LPC family and ADUC70xx
  URL:=http://lpc21isp.sourceforge.net/
  MAINTAINER:=Emil 'Skeen' Madsen <sovende@gmail.com>
endef

define Package/lpc21isp/description
 Portable command line ISP (In-circuit Programmer) for NXP LPC family
 and Analog Devices ADUC70xx.
endef

TARGET_CFLAGS += $(if $(CONFIG_USE_GLIBC),-lc -lgcc_eh)

define Package/lpc21isp/install
		$(INSTALL_DIR) $(1)/usr/sbin
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/lpc21isp $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,lpc21isp))
