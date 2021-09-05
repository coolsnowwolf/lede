#
# Copyright (C) 2016 Nikil Mehta <nikil.mehta@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bitlbee
PKG_VERSION:=3.6
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://get.bitlbee.org/src
PKG_HASH:=9f15de46f29b46bf1e39fc50bdf4515e71b17f551f3955094c5da792d962107e

PKG_MAINTAINER:=Nikil Mehta <nikil.mehta@gmail.com>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:bitlbee:bitlbee

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/bitlbee
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Instant Messaging
  TITLE:=An IRC to other chat networks gateway
  URL:=https://www.bitlbee.org/
  DEPENDS:=+glib2 +libopenssl $(ICONV_DEPENDS) $(INTL_DEPENDS)
endef

define Package/bitlbee/description
	This program can be used as an IRC server which forwards everything you
	say to people on other chat networks: Jabber (which includes Google Talk
	and Facebook Chat), ICQ, AIM, MSN, Yahoo! and Twitter/Identica/Status.net.
endef

define Package/bitlbee/conffiles
/etc/bitlbee/bitlbee.conf
/etc/bitlbee/motd.txt
endef

CONFIGURE_ARGS = \
	--target=$(GNU_TARGET_NAME) \
	--host=$(GNU_TARGET_NAME) \
	--build=$(GNU_HOST_NAME) \
	--prefix=$(CONFIGURE_PREFIX) \
	--etcdir=/etc/bitlbee \
	--ssl=openssl \
	--msn=0

define Build/Install
	$(call Build/Install/Default,install install-etc)
endef

define Package/bitlbee/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/bitlbee $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/bitlbee
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/etc/bitlbee/{bitlbee.conf,motd.txt} $(1)/etc/bitlbee
endef

$(eval $(call BuildPackage,bitlbee))
