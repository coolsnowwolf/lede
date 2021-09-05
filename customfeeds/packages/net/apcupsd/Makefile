#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=apcupsd
PKG_VERSION:=3.14.14
PKG_RELEASE:=3

PKG_MAINTAINER:=Othmar Truniger <github@truniger.ch>
PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/apcupsd
PKG_HASH:=db7748559b6b4c3784f9856561ef6ac6199ef7bd019b3edcd7e0a647bf8f9867
PKG_CPE_ID:=cpe:/a:apc:apcupsd

PKG_BUILD_DEPENDS:=libgd

include $(INCLUDE_DIR)/package.mk

define Package/apcupsd
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libpthread +libusb-compat
  TITLE:=UPS control software
  URL:=http://www.apcupsd.org/
endef

define Package/apcupsd-cgi
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libpthread +libgd
  TITLE:=UPS control software CGI module
  URL:=http://www.apcupsd.org/
endef

CONFIGURE_VARS += SHUTDOWN=/sbin/halt SCRIPTSHELL=/bin/sh WALL=/bin/true APCUPSD_MAIL=/bin/true

define Build/Configure
	$(CP) $(SCRIPT_DIR)/config.* $(PKG_BUILD_DIR)/autoconf/
	$(call Build/Configure/Default, \
		--with-distname=unknown \
		--sysconfdir=/etc/apcupsd \
		--enable-cgi \
		--enable-usb \
		--enable-modbus-usb \
		--without-x \
	)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		LD="$(TARGET_CC)" \
		all install
endef

define Package/apcupsd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/apcupsd $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/smtp $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/apctest $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/apcaccess $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/apcupsd
	$(INSTALL_CONF) ./files/apcupsd.conf $(1)/etc/apcupsd/
	$(INSTALL_CONF) ./files/apcupsd_mail.conf $(1)/etc/apcupsd/
	$(INSTALL_BIN) ./files/changeme $(1)/etc/apcupsd/
	$(INSTALL_BIN) ./files/commfailure $(1)/etc/apcupsd/
	$(INSTALL_BIN) ./files/commok $(1)/etc/apcupsd/
	$(INSTALL_BIN) ./files/offbattery $(1)/etc/apcupsd/
	$(INSTALL_BIN) ./files/onbattery $(1)/etc/apcupsd/
	$(INSTALL_BIN) ./files/apccontrol $(1)/etc/apcupsd/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN)	./files/apcupsd.init $(1)/etc/init.d/apcupsd
endef

define Package/apcupsd-cgi/install
	$(INSTALL_DIR) $(1)/www/cgi-bin/apcupsd
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/etc/apcupsd/*.cgi $(1)/www/cgi-bin/apcupsd
	$(INSTALL_DIR) $(1)/etc/apcupsd
	$(INSTALL_CONF) ./files/apcupsd.css $(1)/etc/apcupsd/
	$(INSTALL_CONF) ./files/hosts.conf $(1)/etc/apcupsd/
	$(INSTALL_CONF) ./files/multimon.conf $(1)/etc/apcupsd/
endef

define Package/apcupsd/conffiles
/etc/apcupsd/apcupsd.conf
/etc/apcupsd/apcupsd_mail.conf
/etc/apcupsd/changeme
/etc/apcupsd/commfailure
/etc/apcupsd/commok
/etc/apcupsd/offbattery
/etc/apcupsd/onbattery
endef

define Package/apcupsd-cgi/conffiles
/etc/apcupsd/apcupsd.css
/etc/apcupsd/hosts.conf
/etc/apcupsd/multimon.conf
endef

$(eval $(call BuildPackage,apcupsd))
$(eval $(call BuildPackage,apcupsd-cgi))
