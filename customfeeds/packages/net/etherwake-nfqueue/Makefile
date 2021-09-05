#
# Copyright (C) 2019 Mister Benjamin <144dbspl@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=etherwake-nfqueue
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/mister-benjamin/etherwake-nfqueue.git
PKG_SOURCE_DATE:=2019-09-28
PKG_SOURCE_VERSION:=f71c269b58585e93575fa3e9fcc1793806fb3080
PKG_MIRROR_HASH:=4960dc592abc4ca06504c92ca09fc736c678353df0dcc32d4081e17b137a9164

PKG_MAINTAINER:=Mister Benjamin <144dbspl@gmail.com>
PKG_LICENSE:=GPL-2.0-or-later

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/etherwake-nfqueue
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libnetfilter-queue +iptables-mod-nfqueue
  TITLE:=Wake up computers on netfilter match
  URL:=https://github.com/mister-benjamin/etherwake-nfqueue
endef

define Package/etherwake-nfqueue/description
 Fork of etherwake with additional support for sending WOL packets
 when a router added a filtered packet to an NFQUEUE.
endef

define Package/etherwake-nfqueue/conffiles
/etc/config/etherwake-nfqueue
endef

define Package/etherwake-nfqueue/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/etherwake-nfqueue $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) files/$(PKG_NAME).config $(1)/etc/config/$(PKG_NAME)
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)
endef

$(eval $(call BuildPackage,etherwake-nfqueue))
