#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=sstp-client
PKG_VERSION:=1.0.15
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/sstp-client/$(PKG_VERSION)
PKG_HASH:=8484aa51fbfbe418a0ebad58ad20a8ee1c46ed71f800be18bcd23b42e6baad64

PKG_MAINTAINER:=Federico Di Marco <fededim@gmail.com>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk

define Package/sstp-client
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=VPN
  DEPENDS=+libevent2 +libopenssl +ppp +resolveip
  TITLE:=SSTP-Client is a SSTP client for Linux.
  URL:=http://sstp-client.sourceforge.net/
endef

define Package/sstp-client/description
The advantage of SSTP compared to other vpn protocols like PPTP and L2TP is that it cannot be easily blocked by firewalls since the traffic is transmitted over HTTPS on port 443.
This software has a similar commandline and configuration to the standard linux pptp-client software.

Features:
- Connect to Microsoft RAS network using SSTP
- Use HTTPS with strong encryption over port 443
- Asynchronous HDLC frame support
- Integration with pon/poff

endef

define Package/sstp-client/conffiles
/etc/ppp/chap-secrets
/etc/ppp/peers/peer-sstp-example-nopty.txt
/etc/ppp/peers/peer-sstp-example.txt
endef

TARGET_CPPFLAGS += -I$(PKG_BUILD_DIR)/include

define Package/sstp-client/install
	$(INSTALL_DIR) $(1)/usr/{bin,lib}
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/.libs/sstpc $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/src/libsstp-api/.libs/*.so* $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/src/pppd-plugin/.libs/*.so* $(1)/usr/lib/

	$(INSTALL_DIR) $(1)/etc
	$(CP) files/etc $(1)
	$(INSTALL_DIR) $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./files/lib/netifd/proto/sstp.sh $(1)/lib/netifd/proto
endef

$(eval $(call BuildPackage,sstp-client))
