#
# Copyright (C) 2007 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=admswconfig
PKG_VERSION:=0.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://downloads.openwrt.org/sources
PKG_HASH:=37b4409ea2105520b4a7327e53c3210251000ddf58f81a8e4c19e6ff9d615afe

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

define Package/admswconfig
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=ADM5120 Switch configuration tool
  DEPENDS:=@TARGET_adm5120
  URL:=http://sharon.esrac.ele.tue.nl/users/pe1rxq/linux-adm/admswconfig/
  MAINTAINER:=Florian Fainelli <florian@openwrt.org>
endef

define Package/admswconfig/description
 A program to configure the internal ethernet switch of an ADM5120 processor.
 You need the corresponding driver for the switch in the kernel. 
 With this program you can configure which ports of the switch belong 
 to the different ethernet devices.
endef

define Build/Configure
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) -Os $(PKG_BUILD_DIR)/admswconfig.c -o $(PKG_BUILD_DIR)/$(PKG_NAME)
endef

define Package/admswconfig/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/admswconfig $(1)/sbin/
	$(INSTALL_DIR) $(1)/lib/network/
	$(INSTALL_DATA) ./files/admswswitch.sh $(1)/lib/network/admswswitch.sh
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/admswconfig $(1)/etc/init.d/admswconfig
endef

$(eval $(call BuildPackage,admswconfig))
