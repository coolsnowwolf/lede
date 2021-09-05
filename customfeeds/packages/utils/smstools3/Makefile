#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=smstools3
PKG_VERSION:=3.1.21
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://smstools3.kekekasvi.com/packages/
PKG_HASH:=a26ba4c02b16f6cf13177bffca6c9230dc5fefaeba8e3030cd4e4905f6a92084
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

PKG_MAINTAINER:=Harald Geyer <harald@ccbib.org>
PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_PARALLEL:=0

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/smstools3
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=SMS Server Tools 3
  DEPENDS:=$(ICONV_DEPENDS)
  URL:=http://smstools3.kekekasvi.com/
endef

define Package/smstools3/description
 The SMS Server Tools 3 is a SMS Gateway software which can send and receive
 short messages through GSM modems and mobile phones.
endef

TARGET_CFLAGS += -D NUMBER_OF_MODEMS=1
TARGET_CFLAGS += $(if $(ICONV_FULL),-D USE_ICONV)
TARGET_CFLAGS += -D DISABLE_INET_SOCKET
TARGET_CFLAGS += -W -Wall
TARGET_CFLAGS += -D_FILE_OFFSET_BITS=64
TARGET_CFLAGS += -fcommon

MAKE_VARS += LFLAGS="$(TARGET_LDFLAGS) $(if $(ICONV_FULL),-liconv)"

define Package/smstools3/configure
endef

define Package/smstools3/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) \
		$(PKG_BUILD_DIR)/src/smsd \
		$(PKG_BUILD_DIR)/scripts/sendsms \
		$(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/smstools3.init $(1)/etc/init.d/smstools3
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_CONF) ./files/smstools3.conf $(1)/etc/smsd.conf
endef

$(eval $(call BuildPackage,smstools3))
