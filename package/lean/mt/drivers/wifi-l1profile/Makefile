#
# Copyright (C) 2016 MediaTek
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/version.mk

PKG_NAME:=wifi-l1profile
PKG_RELEASE:=1
PKG_BUILD_DEPENDS:=
PKG_FILE_DEPENDS:=
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/wifi-l1profile
  SECTION:=Kernel modules
  CATEGORY:=Kernel modules
  SUBMENU:=Wireless Drivers
  DEPENDS:=
  TITLE:=Build WiFi l1profile.data on demand.
  VERSION:=$(PKG_RELEASE)-$(REVISION)
  MENU:=1
endef

define Package/wifi-l1profile/description
  This package helps to build l1profile on demand.
endef

define Package/wifi-l1profile/config
	if PACKAGE_wifi-l1profile
	source "$(SOURCE)/l1profile.config.in"
	endif
endef

define Build/Prepare
endef

define Build/Compile
	rm -rf $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)
	python ./make-l1profile.py $(TOPDIR)/.config /dev/null
endef

define Package/wifi-l1profile/install
	$(INSTALL_DIR) $(1)/etc/wireless/
	python ./make-l1profile.py $(TOPDIR)/.config $(1)/etc/wireless/l1profile.dat > /dev/null
endef

$(eval $(call BuildPackage,wifi-l1profile))
