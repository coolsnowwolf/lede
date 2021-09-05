#
# Copyright (C) 2009-2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=sslh
PKG_VERSION:=v1.21c
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://rutschle.net/tech/sslh/
PKG_HASH:=3bfe783726f82c1f5a4be630ddc494ebb08dbb69980662cd7ffdeb7bc9e1e706

PKG_MAINTAINER:=Jonathan McCrohan <jmccrohan@gmail.com>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk

define Package/sslh
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Routing and Redirection
  TITLE:=SSL/SSH multiplexer
  DEPENDS:=+libconfig +libcap +USE_UCLIBC:libpcre +USE_MUSL:libpcre
  URL:=https://rutschle.net/tech/sslh/README.html
endef

define Package/sslh/conffiles
/etc/config/sslh
/etc/sslh.conf
endef

MAKE_FLAGS += \
  USELIBCAP=1 \
  $(if $(CONFIG_USE_GLIBC),USELIBPCRE=,USELIBPCRE=1)

define Package/sslh/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sslh-fork $(1)/usr/sbin/sslh
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) files/$(PKG_NAME).config $(1)/etc/config/$(PKG_NAME)
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/basic.cfg $(1)/etc/sslh.conf
endef

$(eval $(call BuildPackage,sslh))
