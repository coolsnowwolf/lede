#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=iftop
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://code.blinkace.com/pdw/iftop.git
PKG_SOURCE_DATE:=2017-03-22
PKG_SOURCE_VERSION:=949ed0f7e2c54c598868c270b82c2d702131a339
PKG_MIRROR_HASH:=2ba96d9a2adf4e43aaab439a9ccab8f21b415da48d1c8939f6dcea8e6e11524f
PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>
PKG_LICENSE:=GPL-2.0

PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

define Package/iftop
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libpcap +libncurses +libpthread
  TITLE:=display bandwith usage on an interface
  URL:=http://www.ex-parrot.com/~pdw/iftop/
endef

define Package/iftop/description
	iftop does for network usage what top(1) does for CPU usage. It 
	listens to network traffic on a named interface and displays a 
	table of current bandwidth usage by pairs of hosts. Handy for 
	answering the question 'why is our ADSL link so slow?'.
endef

define Package/iftop/install	
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/iftop $(1)/usr/bin/
endef

$(eval $(call BuildPackage,iftop))
