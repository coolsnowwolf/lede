#
# Copyright (C) 2010-2014 OpenWrt.org
# Copyright (C) 2010 segal.di.ubi.pt
# Copyright (C) 2016 Karol Babioch <karol@babioch.de>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=wifitoggle
PKG_VERSION:=1
PKG_RELEASE:=6
PKG_LICENSE:=GPL-2.0-or-later

PKG_MAINTAINER:=Nuno Goncalves <nunojpg@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/wifitoggle
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Script to toggle Wi-Fi with a button and UCI config
  PKGARCH:=all
endef

define Package/wifitoggle/description
Very versatile script to toggle Wi-Fi with a button. Allows to set
timeouts, persist changes after boot, and set LEDs according to the state.
endef

define Package/wifitoggle/conffiles
/etc/config/wifitoggle
endef

define Build/Compile
endef

define Package/wifitoggle/install
	$(INSTALL_DIR) $(1)/etc/hotplug.d/button
	$(INSTALL_BIN) ./files/wifitoggle.hotplug $(1)/etc/hotplug.d/button/50-wifitoggle
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/wifitoggle.config $(1)/etc/config/wifitoggle
endef

$(eval $(call BuildPackage,wifitoggle))
