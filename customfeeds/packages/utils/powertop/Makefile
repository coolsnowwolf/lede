#
# Copyright (C) 2019 Lucian Cristain <lucian.cristian@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=powertop
PKG_VERSION:=2.10
PKG_RELEASE:=5

PKG_SOURCE:=$(PKG_NAME)-v$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://01.org/sites/default/files/downloads/
PKG_HASH:=d3b7459eaba7d01c8841dd33a3b4d369416c01e9bd8951b0d88234cf18fe4a75

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-v$(PKG_VERSION)
PKG_MAINTAINER:=Lucian Cristain <lucian.cristian@gmail.com>
PKG_LICENSE:=GPL-2.0

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/powertop
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=$(INTL_DEPENDS) +libstdcpp +libpci +libncursesw +libnl-genl
  TITLE:=Power consumption monitor
  URL:=https://01.org/powertop
endef

define Package/powertop/description
 PowerTOP is a Linux tool to diagnose issues with power consumption
 and power management.
endef

TARGET_LDFLAGS += \
	$(if $(INTL_FULL),-lintl) \
	$(if $(CONFIG_USE_GLIBC),-lm) \
	$(if $(CONFIG_USE_GLIBC),-lpthread)

define Package/powertop/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/powertop \
		$(1)/usr/sbin/
endef

$(eval $(call BuildPackage,powertop))
