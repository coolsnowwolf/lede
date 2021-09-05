#
# Copyright (C) 2017 Yousong Zhou <yszhou4tech@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=udns
PKG_VERSION:=0.4
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=115108dc791a2f9e99e150012bcb459d9095da2dd7d80699b584ac0ac3768710
PKG_SOURCE_URL:=http://www.corpit.ru/mjt/udns
PKG_LICENSE:=LGPL-2.1
PKG_MAINTAINER:=Yousong Zhou <yszhou4tech@gmail.com>

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libudns
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=DNS Resolver Library
  URL:=http://www.corpit.ru/mjt/udns.html
endef

define Package/libudns/description
  UDNS is a stub DNS resolver library with ability to perform both syncronous
  and asyncronous DNS queries.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

define Package/libudns/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libudns.so* $(1)/usr/lib/
endef

define udns-utility-template
  define Package/udns-$(1)
    SECTION:=utils
    CATEGORY:=Utilities
    TITLE:= $(2)
    URL:=http://www.corpit.ru/mjt/udns.html
    DEPENDS:=+libudns
  endef

  define Package/udns-$(1)/install
	$(INSTALL_DIR) $$(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/$(1) $$(1)/usr/bin/
  endef

endef

CONFIGURE_ARGS += $(DISABLE_IPV6)

$(eval $(call udns-utility-template,dnsget,a simple DNS query tool))
$(eval $(call udns-utility-template,rblcheck,a simple DNSBL lookups tool))
$(eval $(call udns-utility-template,ex-rdns,a parallel rDNS resolver))

$(eval $(call BuildPackage,libudns))
$(eval $(call BuildPackage,udns-dnsget))
$(eval $(call BuildPackage,udns-rblcheck))
$(eval $(call BuildPackage,udns-ex-rdns))
