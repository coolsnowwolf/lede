#
# Copyright (C) 2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libnetconf2
PKG_VERSION:=1.1.43
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/CESNET/libnetconf2/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=66139fc9e68aa89c82235f4135dba9e44f5db663541279c14c74131e22b7f571

PKG_MAINTAINER:=Jakov Smolic <jakov.smolic@sartura.hr>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE

CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libnetconf2
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=NETCONF library
  URL:=https://github.com/CESNET/libnetconf2
  DEPENDS:= +libyang +libssh +libopenssl +libpthread
endef

define Package/libnetconf2/description
 libnetconf2 is the NETCONF library in C intended for building NETCONF clients and servers.

 libnetconf2 provides basic functions to connect NETCONF client and server to each other via
 SSH, to send and receive NETCONF messages. NETCONF datastore implementation is not included.
endef

define Package/libnetconf2/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnetconf2.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libnetconf2))
