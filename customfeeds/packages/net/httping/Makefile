#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=httping
PKG_VERSION:=2.5
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tgz
PKG_SOURCE_URL:=http://www.vanheusden.com/httping
PKG_HASH:=3e895a0a6d7bd79de25a255a1376d4da88eb09c34efdd0476ab5a907e75bfaf8

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/httping/Default
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Httping is like 'ping' but for http-requests
  URL:=http://www.vanheusden.com/httping/
  DEPENDS:=$(INTL_DEPENDS)
  MAINTAINER:=Marcin Jurkowski <marcin1j@gmail.com>
endef

define Package/httping/Default/description
	Give it an url, and it'll show you how long it takes to connect, send a
	request and retrieve the reply (only the headers). Be aware that the
	transmission across the network also takes time!
endef

define Package/httping
  $(call Package/httping/Default)
  DEPENDS+= +libopenssl
  TITLE+= (with SSL support)
  VARIANT:=ssl
endef

define Package/httping/description
	$(call Package/httping/Default/description)
	This package is built with SSL support.
endef

define Package/httping-nossl
  $(call Package/httping/Default)
  TITLE+= (without SSL support)
  VARIANT:=nossl
endef

define Package/httping-nossl/description
	$(call Package/httping/Default/description)
	This package is built without SSL support.
endef

define Build/Configure
endef

TARGET_LDFLAGS += $(INTL_LDFLAGS) $(if $(INTL_FULL),-lintl)

MAKE_FLAGS += \
	DEBUG="no" \
	FW="no" \
	NC="no" \
	TFO="no" \
	JSON="no" \
	NAGIOS="no" \
	HELP="yes" \
	FULLHELP="no"


ifeq ($(BUILD_VARIANT),nossl)
	MAKE_FLAGS += SSL="no"
endif

define Package/httping/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/httping $(1)/usr/bin/
endef

Package/httping-nossl/install = $(Package/httping/install)

$(eval $(call BuildPackage,httping))
$(eval $(call BuildPackage,httping-nossl))
