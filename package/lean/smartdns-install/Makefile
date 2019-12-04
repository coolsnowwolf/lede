#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=smartdns-install
PKG_VERSION:=2

APP_NAME:=smartdns
APP_VERSION:=1.2019.11.02-1102
RELEASES_VERSION:=release27

LUCI_NAME:=luci-app-smartdns
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  CATEGORY:=LuCI
  SUBMENU:=3. Applications
  TITLE:=Automatically install smartdns
  PKGARCH:=all
  URL:=https://github.com/pymumu/smartdns
endef

define Package/$(PKG_NAME)/description
endef

ifeq ($(ARCH),x86_64)
	PKG_ARCH_SMARTDNS:=x86_64
endif
ifeq ($(ARCH),mipsel)
	PKG_ARCH_SMARTDNS:=mipsel
endif
ifeq ($(ARCH),mips)
	PKG_ARCH_SMARTDNS:=mips
endif
ifeq ($(ARCH),i386)
	PKG_ARCH_SMARTDNS:=x86
endif
ifeq ($(ARCH),arm)
	PKG_ARCH_SMARTDNS:=arm
endif
ifeq ($(ARCH),aarch64)
	PKG_ARCH_SMARTDNS:=aarch64
endif

define Build/Prepare
	[ ! -f $(PKG_BUILD_DIR)/$(APP_NAME).$(APP_VERSION).$(PKG_ARCH_SMARTDNS)-openwrt-all.ipk ] && wget https://github.com/pymumu/smartdns/releases/download/$(RELEASES_VERSION)/$(APP_NAME).$(APP_VERSION).$(PKG_ARCH_SMARTDNS)-openwrt-all.ipk -O $(PKG_BUILD_DIR)/$(APP_NAME).$(APP_VERSION).$(PKG_ARCH_SMARTDNS)-openwrt-all.ipk
	[ ! -f $(PKG_BUILD_DIR)/$(LUCI_NAME).$(APP_VERSION).all-luci-all.ipk ] && wget https://github.com/pymumu/smartdns/releases/download/$(RELEASES_VERSION)/$(LUCI_NAME).$(APP_VERSION).all-luci-all.ipk -O $(PKG_BUILD_DIR)/$(LUCI_NAME).$(APP_VERSION).all-luci-all.ipk
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/share/smartdns-install
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/$(APP_NAME).$(APP_VERSION).$(PKG_ARCH_SMARTDNS)-openwrt-all.ipk $(1)/usr/share/smartdns-install/smartdns.ipk
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/$(LUCI_NAME).$(APP_VERSION).all-luci-all.ipk $(1)/usr/share/smartdns-install/luci-app-smartdns.ipk
	$(INSTALL_BIN) ./root/etc/smartdns-install.sh $(1)/usr/share/smartdns-install/smartdns-install.sh
	$(INSTALL_CONF) ./root/etc/uci-defaults/* $(1)/etc/uci-defaults
endef

define Package/$(PKG_NAME)/postinst
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
