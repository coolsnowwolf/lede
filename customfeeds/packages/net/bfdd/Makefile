#
# Copyright (C) 2019 Lucian Cristian <lucian.cristian@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=bfdd
PKG_SOURCE_DATE:=2019-08-22
PKG_RELEASE:=4

PKG_SOURCE_VERSION:=c54534beb524afc3972039f57b56ec65332b43f7
PKG_SOURCE_URL:=https://codeload.github.com/rzalamena/bfdd/tar.gz/$(PKG_SOURCE_VERSION)?
PKG_SOURCE:=$(PKG_NAME)-$(PKG_SOURCE_DATE).tar.gz
PKG_HASH:=8b65f502163aacfe43bb897464f3bf44bc5af4cc85d23b7c644e329abf89cc5f

PKG_MAINTAINER:=Lucian Cristian <lucian.cristian@gmail.com>
PKG_LICENSE:=GPL-2.0-or-later

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_SOURCE_VERSION)
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/bfdd
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Routing and Redirection
  TITLE:=BFD daemon
  URL:=https://github.com/rzalamena/bfdd
  DEPENDS:=+libevent2 +libjson-c
endef

define Package/bfdd/description
  A port of Cumulus BFD daemon to a more portable daemon.

  Bidirectional Forwarding Detection (BFD) is a network protocol that is used to
  detect faults between two forwarding engines connected by a link. It provides
  low-overhead detection of faults even on physical media that doesn't support
  failure detection of any kind, such as Ethernet, virtual circuits, tunnels and
  MPLS Label Switched Paths.
endef

define Package/bfdd/conffiles
/etc/bfdd/bfdd.json
endef

define Package/bfdd/install
	$(INSTALL_DIR) \
	 $(1)/usr/sbin \
	 $(1)/etc/bfdd \
	 $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/{bfdd,bfdctl} $(1)/usr/sbin/
	$(INSTALL_BIN) ./files/bfdd.init $(1)/etc/init.d/bfdd
	$(INSTALL_CONF) ./files/bfdd.template.json $(1)/etc/bfdd/bfdd.json
endef

$(eval $(call BuildPackage,bfdd))
