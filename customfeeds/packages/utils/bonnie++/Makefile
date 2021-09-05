#
# Copyright (C) 2009-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bonnie++
PKG_VERSION:=2.00a
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tgz
PKG_SOURCE_URL:=https://www.coker.com.au/bonnie++
PKG_HASH:=a8d33bbd81bc7eb559ce5bf6e584b9b53faea39ccfb4ae92e58f27257e468f0e

PKG_MAINTAINER:=
PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=copyright.txt

PKG_INSTALL:=1
PKG_BUILD_DEPENDS:=meson/host

include $(INCLUDE_DIR)/package.mk
include ../../devel/meson/meson.mk

define Package/bonniexx
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libstdcpp +libpthread
  TITLE:=Bonnie++ - hard drive bottleneck testing program.
  URL:=https://www.coker.com.au/bonnie++/
endef

define Package/bonniexx/description
  Bonnie++ is a benchmark suite that is aimed at performing a number of simple
  tests of hard drive and file system performance.
endef

MESON_ARGS += \
	-Db_lto=true \
	-Dcpp_eh=none \
	-Dcpp_rtti=false

TARGET_CXXFLAGS += -ffunction-sections -fdata-sections

define Package/bonniexx/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/bonnie++ $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/bon_csv2html $(1)/usr/bin/
endef

$(eval $(call BuildPackage,bonniexx))
