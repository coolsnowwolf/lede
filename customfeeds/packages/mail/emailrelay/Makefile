#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=emailrelay
PKG_VERSION:=2.2
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-src.tar.gz
PKG_SOURCE_URL:=@SF/emailrelay/$(PKG_VERSION)
PKG_HASH:=41da9f4356eb6c6935a32f2940edb7af04b202e531c1028f700b93a36a03b0ac

PKG_MAINTAINER:=Federico Di Marco <fededim@gmail.com>
PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_CONFIG_DEPENDS:=CONFIG_EMAILRELAY_SUPPORT_VERBOSE_DBG CONFIG_EMAILRELAY_SSL
PKG_FORTIFY_SOURCE:=0 #GCC8 and 9 fix

include $(INCLUDE_DIR)/package.mk

define Package/emailrelay
  SECTION:=mail
  CATEGORY:=Mail
  DEPENDS:=+EMAILRELAY_SSL:libopenssl +libstdcpp
  TITLE:=A simple SMTP proxy and MTA
  URL:=http://emailrelay.sourceforge.net/
  MENU:=1
endef


define Package/emailrelay/config
config EMAILRELAY_SUPPORT_VERBOSE_DBG
	bool "Enable support for extended logging"
	depends on PACKAGE_emailrelay
	default n
	help
	  Enables support for extended logging (must also be explicitely enabled by using command line switch --debug when starting emailrelay)

config EMAILRELAY_SSL
	bool "Enable support for OpenSSL"
	depends on PACKAGE_emailrelay
	default y
	select PACKAGE_libopenssl
	help
 	  Builds the package with OpenSSL support (SSMTP is supported).
endef



define Package/emailrelay/description
 Emailrelay is a simple SMTP proxy and store-and-forward message transfer agent (MTA).

 When running as a proxy all e-mail messages can be passed through
 a user-defined program, such as a spam filter, which can drop,
 re-address or edit messages as they pass through. When running
 as a store-and-forward MTA incoming messages are stored in a
 local spool directory, and then forwarded to the next SMTP
 server on request.
endef


define Package/emailrelay/conffiles
/etc/config/emailrelay
/etc/emailrelay.auth
endef


CONFIGURE_ARGS += \
	--without-doxygen \
	--without-man2html \
	--without-mbedtls \
	--without-pam \
	--disable-bsd \
	--disable-gui \
	--disable-mac \
	--disable-testing \
	--disable-windows

ifeq ($(CONFIG_EMAILRELAY_SSL),y)
	CONFIGURE_ARGS += \
		--with-openssl
else
	CONFIGURE_ARGS += \
		--without-openssl
endif

ifeq ($(CONFIG_EMAILRELAY_SUPPORT_VERBOSE_DBG),y)
	CONFIGURE_ARGS += \
		--enable-debug=yes
endif

TARGET_CXXFLAGS += -ffunction-sections -fdata-sections -flto
TARGET_CXXFLAGS += -Wl,--gc-sections,--as-needed

define Package/emailrelay/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/main/emailrelay $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/main/emailrelay-filter-copy $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/main/emailrelay-passwd $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/main/emailrelay-submit $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) files/$(PKG_NAME).auth $(1)/etc/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) files/$(PKG_NAME).config $(1)/etc/config/$(PKG_NAME)
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)
endef


$(eval $(call BuildPackage,emailrelay))
