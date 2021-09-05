#
# Copyright (C) 2010-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=chaosvpn

PKG_VERSION:=2.19
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/ryd/chaosvpn/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=53625d131140529e88d8a14c34cc4d8d5d0134292d90f4ae55e9df29d3232828
PKG_LICENSE:=Apache-2.0
PKG_MAINTAINER:=Norbert Summer <git@o-g.at>

include $(INCLUDE_DIR)/package.mk

define Package/chaosvpn
		TITLE:=Config generator for chaos vpn
		SECTION:=net
		CATEGORY:=Network
		URL:=https://wiki.hamburg.ccc.de/index.php/ChaosVPN::DebianHowto
		SUBMENU:=VPN
		DEPENDS:=+tinc +ip +kmod-tun +libopenssl +zlib
endef

define Package/chaosvpn/description
		Configure tincd for the chaosvpn network
endef

define Package/chaosvpn/conffiles
/etc/tinc/chaosvpn.conf
endef

MAKE_FLAGS += \
		COPT="$(TARGET_CFLAGS) $(TARGET_CPPFLAGS) $(TARGET_LDFLAGS)"

define Package/chaosvpn/install
		$(INSTALL_DIR) $(1)/etc/tinc/chaos/ $(1)/usr/sbin/ $(1)/etc/init.d/ $(1)/etc/hotplug.d/iface/
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/chaosvpn $(1)/usr/sbin/
		$(INSTALL_BIN) ./files/chaosvpn.init $(1)/etc/init.d/chaosvpn
		$(INSTALL_BIN) ./files/chaosvpn.hotplug $(1)/etc/hotplug.d/iface/40-chaosvpn
		$(INSTALL_CONF) $(PKG_BUILD_DIR)/chaosvpn.conf $(1)/etc/tinc/chaosvpn.conf
endef

$(eval $(call BuildPackage,chaosvpn))

