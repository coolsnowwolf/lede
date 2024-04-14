#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=vsftpd-alt
PKG_VERSION:=3.0.5
PKG_RELEASE:=2

PKG_SOURCE:=vsftpd-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://security.appspot.com/downloads/
PKG_HASH:=26b602ae454b0ba6d99ef44a09b6b9e0dfa7f67228106736df1f278c70bc91d3
PKG_LICENSE:=GPLv2
PKG_CPE_ID:=cpe:/a:vsftpd_project:vsftpd

BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DIR:=$(BUILD_DIR)/vsftpd-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/vsftpd-alt/Default
  SUBMENU:=File Transfer
  SECTION:=net
  CATEGORY:=Network
  TITLE:=A fast and secure FTP server (with aio support)
  URL:=http://vsftpd.beasts.org/
  MAINTAINER:=Cezary Jackiewicz <cezary@eko.one.pl>
  DEPENDS=+libuci 
  #+PACKAGE_libpam:libpam 
endef

TARGET_CFLAGS += -ffunction-sections -fdata-sections -flto
TARGET_LDFLAGS += -Wl,--gc-sections,--as-needed,--no-undefined,--no-allow-shlib-undefined

define Package/vsftpd-alt/conffiles
/etc/vsftpd.conf
endef

define Package/vsftpd-alt/config
config VSFTPD_USE_UCI_SCRIPTS
	bool "Uses UCI scripts"
	depends on PACKAGE_vsftpd-alt
	default y
endef

ifneq ($(CONFIG_USE_MUSL),)
  NLSSTRING:=-lcrypt
  TARGET_CFLAGS += -D_LARGEFILE64_SOURCE
else ifneq ($(CONFIG_USE_GLIBC),)
  NLSSTRING:=-lcrypt
else
  NLSSTRING:=-lcrypt -lnsl
endif

TARGET_CFLAGS += -D_GNU_SOURCE -include fcntl.h

define Package/vsftpd-alt
$(call Package/vsftpd-alt/Default)
endef

define Build/Compile
	$(SED) 's/-lcrypt -lnsl/$(NLSSTRING)/' $(PKG_BUILD_DIR)/Makefile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) $(TARGET_CPPFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		vsftpd
endef

ifeq ($(CONFIG_VSFTPD_USE_UCI_SCRIPTS),y)
define Package/vsftpd-alt/install
$(call Package/vsftpd-alt/install/default, $(1))
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/config
ifdef CONFIG_PACKAGE_libpam
	$(INSTALL_DIR) $(1)/etc/pam.d
endif
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/vsftpd $(1)/usr/sbin
	$(INSTALL_BIN) ./files/vsftpd-uci.init $(1)/etc/init.d/vsftpd
	$(INSTALL_BIN) ./files/vsftpd.uci $(1)/etc/config/vsftpd
	$(INSTALL_BIN) ./files/vsftpd_wrapper $(1)/usr/sbin
	$(INSTALL_BIN) ./files/vsftpd_prepare $(1)/usr/sbin
endef
else
define Package/vsftpd-alt/install
$(call Package/vsftpd-alt/install/default, $(1))
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_CONF) ./files/vsftpd.conf $(1)/etc/vsftpd.conf
	$(INSTALL_BIN) ./files/vsftpd.init $(1)/etc/init.d/vsftpd
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/vsftpd $(1)/usr/sbin	
endef
endif

$(eval $(call BuildPackage,vsftpd-alt))
