#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=netdiscover
PKG_VERSION:=0.3-pre-beta7
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-LINUXONLY.tar.gz
PKG_SOURCE_URL:=@SF/netdiscover
PKG_HASH:=01c6e090c3b06e374005f7efcead3b5b2f63f47bfb94383c1dbde9abcf1cd8aa
PKG_MAINTAINER:=Mislav Novakovic <mislav.novakovic@sartura.hr>
PKG_LICENSE:=GPL-2.0

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/netdiscover
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libpcap +libnet-1.2.x +libpthread
  TITLE:=An active/passive address reconnaissance tool
  URL:=https://sourceforge.net/projects/netdiscover
endef

define Package/netdiscover/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/netdiscover $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,netdiscover))
