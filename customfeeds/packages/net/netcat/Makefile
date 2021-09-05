#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=netcat
PKG_VERSION:=0.7.1
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=@SF/$(PKG_NAME)
PKG_HASH:=b55af0bbdf5acc02d1eb6ab18da2acd77a400bafd074489003f3df09676332bb
PKG_MAINTAINER:=Adam Gensler <openwrt@a.gnslr.us>
PKG_LICENSE:=GPL-2.0
PKG_CPE_ID:=cpe:/a:netcat:netcat
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/netcat
  SECTION:=net
  CATEGORY:=Network
  TITLE:=A feature-rich network debugging and exploration tool.
  URL:=http://netcat.sourceforge.net/
endef

define Package/netcat/description
		Netcat is a featured networking utility which reads and writes data across network connections, using the TCP/IP protocol.
	It is designed to be a reliable "back-end" tool that can be used directly or easily driven by other programs and scripts. At the same time, it is a feature-rich network debugging and exploration tool, since it can create almost any kind of connection you would need and has several interesting built-in capabilities.
endef

define Build/Configure
	$(call Build/Configure/Default, \
		--disable-rpath \
		--with-included-getopt \
	)
endef

define Package/netcat/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/netcat \
		$(1)/usr/bin
endef

define Package/netcat/postinst
#!/bin/sh
if [ -e $${IPKG_INSTROOT}/usr/bin/nc ]; then
  rm -rf $${IPKG_INSTROOT}/usr/bin/nc;
fi
ln -s ./netcat $${IPKG_INSTROOT}/usr/bin/nc
endef

define Package/netcat/postrm
#!/bin/sh
rm $${IPKG_INSTROOT}/usr/bin/nc
ln -s ../../bin/busybox $${IPKG_INSTROOT}/usr/bin/nc
$${IPKG_INSTROOT}/usr/bin/nc 2>&1 | grep 'applet not found' > /dev/null 2>&1 && rm $${IPKG_INSTROOT}/usr/bin/nc
exit 0
endef


$(eval $(call BuildPackage,netcat))
