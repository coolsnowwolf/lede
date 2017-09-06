#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=emailrelay
PKG_VERSION:=1.9
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-src.tar.gz
PKG_SOURCE_URL:=@SF/emailrelay/$(PKG_VERSION)
PKG_MD5SUM:=0892fbf993407c6b5a16f96e23299b62

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)


include $(INCLUDE_DIR)/uclibc++.mk
include $(INCLUDE_DIR)/package.mk

define Package/emailrelay/Default
  SECTION:=net
  SUBMENU:=Web Servers/Proxies
  CATEGORY:=Network
  DEPENDS:=$(CXX_DEPENDS)
  TITLE:=A simple SMTP proxy and MTA
  URL:=http://emailrelay.sourceforge.net/
  MAINTAINER := Federico Di Marco <fededim@gmail.com>
endef


define Package/emailrelay
$(call Package/emailrelay/Default)
  TITLE+= (with OpenSSL support)
  DEPENDS+=+libopenssl
  VARIANT:=ssl
endef

define Package/emailrelay-nossl
$(call Package/emailrelay/Default)
  TITLE+= (no SSL support)
  VARIANT:=nossl
endef



define Package/emailrelay/Default/description
 Emailrelay is a simple SMTP proxy and store-and-forward message transfer agent (MTA). 
 
 When running as a proxy all e-mail messages can be passed through 
  a user-defined program, such as a spam filter, which can drop, 
  re-address or edit messages as they pass through. When running 
  as a store-and-forward MTA incoming messages are stored in a 
  local spool directory, and then forwarded to the next SMTP 
  server on request.
endef


define Package/emailrelay/description
$(call Package/emailrelay/Default/description)

 This package is built with OpenSSL support (SSMTP is supported).
endef

define Package/emailrelay-nossl/description
$(call Package/emailrelay/Default/description)

 This package is built without SSL support (no SSMTP)
endef


define Package/emailrelay/conffiles
/etc/emailrelay.auth
endef

define Package/emailrelay-nossl/conffiles
/etc/emailrelay.auth
endef

CONFIGURE_ARGS += \
	--with-pam=no

CONFIGURE_VARS += \
	CXXFLAGS="$$$$CXXFLAGS -fno-rtti"	

ifeq ($(BUILD_VARIANT),ssl)
	CONFIGURE_ARGS += \
		--with-openssl
endif

ifeq ($(BUILD_VARIANT),nossl)
	CONFIGURE_ARGS += \
		--with-openssl=no
endif

define Package/emailrelay/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/main/emailrelay $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/main/emailrelay-filter-copy $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/main/emailrelay-passwd $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/main/emailrelay-poke $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/main/emailrelay-submit $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) files/$(PKG_NAME).auth $(1)/etc/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)
endef

Package/emailrelay-nossl/install = $(Package/emailrelay/install)


$(eval $(call BuildPackage,emailrelay))
$(eval $(call BuildPackage,emailrelay-nossl))
