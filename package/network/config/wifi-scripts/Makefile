#
# Copyright (C) 2024 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=wifi-scripts
PKG_VERSION:=1.0
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>
PKG_CONFIG_DEPENDS:=CONFIG_WIFI_SCRIPTS_UCODE

include $(INCLUDE_DIR)/package.mk

define Package/wifi-scripts
  SECTION:=utils
  CATEGORY:=Base system
  DEPENDS:=+netifd +ucode +ucode-mod-nl80211 +ucode-mod-rtnl +ucode-mod-ubus +ucode-mod-uci +ucode-mod-digest
  TITLE:=Wi-Fi configuration scripts
  PKGARCH:=all
endef

define Package/wifi-scripts/config
	source "$(SOURCE)/Config.in"
endef

define Package/wifi-scripts/description
 A set of scripts that handle setup and configuration of Wi-Fi devices.
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/wifi-scripts/install
	$(INSTALL_DIR) $(1)
	$(CP) ./files/* $(1)/
ifeq ($(CONFIG_WIFI_SCRIPTS_UCODE),y)
	$(CP) ./files-ucode/* $(1)/
endif
ifeq ($(CONFIG_WIFI_SCRIPTS_WPS_BUTTON_DISABLED),y)
	rm -f $(1)/etc/rc.button/wps
endif
endef

$(eval $(call BuildPackage,wifi-scripts))
