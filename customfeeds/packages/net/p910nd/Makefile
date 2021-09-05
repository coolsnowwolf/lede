#
# Copyright (C) 2009-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=p910nd
PKG_VERSION:=0.97
PKG_RELEASE:=9

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=@SF/p910nd
PKG_LICENSE:=GPLv2
PKG_LICENSE_FILES:=COPYING
PKG_HASH:=4ac980a3ae24babae6f70f0a692625ece03a4a92c357fbb10d2e368386c3c26f
PKG_MAINTAINER:=Philipp Kerling <pkerling@casix.org>

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/p910nd
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Printing
  TITLE:=A small non-spooling printer server
  URL:=http://p910nd.sourceforge.net
  USERID:=p910nd=393:lp=7
endef

define Package/p910nd/conffiles
/etc/config/p910nd
endef

define Package/p910nd/description
 p910nd is a small daemon that copies any data received on
 the port it is listening on to the corresponding printer
 port. It is primarily intended for diskless Linux hosts
 running as printer drivers but there is no reason why it
 could not be used on diskful hosts. Port 9100 is copied
 to /dev/lp0, 9101 to /dev/lp1 and 9102 to /dev/lp2. The
 default is port 9100 to /dev/lp0.
endef

MAKE_FLAGS += \
	CFLAGS="$(TARGET_CFLAGS) -DLOCKFILE_DIR=\"\\\"/tmp\"\\\""

define Package/p910nd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/p910nd $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/p910nd.config $(1)/etc/config/p910nd
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/p910nd.init $(1)/etc/init.d/p910nd
	$(INSTALL_DIR) $(1)/etc/hotplug.d/usbmisc
	$(INSTALL_BIN) ./files/p910nd.hotplug $(1)/etc/hotplug.d/usbmisc/20-p910nd
endef

$(eval $(call BuildPackage,p910nd))
