#
# Copyright (C) 2016-2019 Jason A. Donenfeld <Jason@zx2c4.com>
# Copyright (C) 2016 Baptiste Jonglez <openwrt@bitsofnetworks.org>
# Copyright (C) 2016-2017 Dan Luedtke <mail@danrl.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=wireguard-tools

PKG_VERSION:=1.0.20200820
PKG_RELEASE:=1

PKG_SOURCE:=wireguard-tools-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://git.zx2c4.com/wireguard-tools/snapshot/
PKG_HASH:=7735a04c68fffb101a10a67e3bd97a171f2b8eb47e9ddce2be68eb6538b013d0

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/package-defaults.mk

MAKE_PATH:=src
MAKE_VARS += PLATFORM=linux

define Package/wireguard-tools
  $(call Package/wireguard/Default)
  TITLE:=WireGuard userspace control program (wg)
  DEPENDS:=+ip
endef

define Package/wireguard-tools/description
  $(call Package/wireguard/Default/description)

  This package provides the userspace control program for WireGuard,
  `wg(8)`, a netifd protocol helper, and a re-resolve watchdog script.
endef

define Package/wireguard-tools/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/wg $(1)/usr/bin/
	$(INSTALL_BIN) ./files/wireguard_watchdog $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/lib/netifd/proto/
	$(INSTALL_BIN) ./files/wireguard.sh $(1)/lib/netifd/proto/
endef

$(eval $(call BuildPackage,wireguard-tools))
