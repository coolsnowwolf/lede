#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=pen
PKG_VERSION:=0.34.1
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://siag.nu/pub/pen/
PKG_HASH:=2b640795029df9d1672e17202c109cc5d42538f6754a6070dc27da640881e864

PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING
PKG_MAINTAINER:=

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/pen
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libopenssl
  TITLE:=Simple TCP load balancer
  URL:=http://siag.nu/pen/
endef

define Package/pen/description
	This is pen, a load balancer for "simple" TCP based protocols
	such as HTTP or SMTP. It allows several servers to appear as
	one to the outside and automatically detects servers that are
	down and distributes clients among the available servers.
	This gives high availability and scalable performance.
endef

CONFIGURE_ARGS += \
		--with-poll \
		--with-ssl="$(STAGING_DIR)/usr" \
		--without-geoip \

define Package/pen/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/mergelogs $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/pen $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/penctl $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/penlog $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/penlogd $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/penctl.cgi $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/penstats $(1)/usr/bin/
endef

$(eval $(call BuildPackage,pen))
