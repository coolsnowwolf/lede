#
# Copyright (C) 2008-2012 OpenWrt.org
# Copyright (C) 2017 Willem van den Akker <wvdakker@wilsoft.nl>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=shorewall-lite
PKG_MAJOR_MINOR_VERSION:=5.2
PKG_BUGFIX_MAJOR_VERSION:=8
PKG_BUGFIX_MINOR_VERSION:=
PKG_VERSION:=$(PKG_MAJOR_MINOR_VERSION).$(PKG_BUGFIX_MAJOR_VERSION)$(PKG_BUGFIX_MINOR_VERSION)
PKG_DIRECTORY:=$(PKG_MAJOR_MINOR_VERSION).$(PKG_BUGFIX_MAJOR_VERSION)
PKG_RELEASE:=1

PKG_SOURCE_URL:=http://shorewall.org/pub/shorewall/$(PKG_MAJOR_MINOR_VERSION)/shorewall-$(PKG_DIRECTORY)/ \
	http://slovakia.shorewall.net/pub/shorewall/$(PKG_MAJOR_MINOR_VERSION)/shorewall-$(PKG_DIRECTORY)/ \
	http://www.shorewall.no/pub/shorewall/$(PKG_MAJOR_MINOR_VERSION)/shorewall-$(PKG_DIRECTORY)/ \
	http://france.shorewall.net/$(PKG_MAJOR_MINOR_VERSION)/shorewall-$(PKG_DIRECTORY)/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_HASH:=b6ebf750f9501818da0967c80c3d6f08cede5531e48b23ebbf241a2d1fad2cfd

PKG_MAINTAINER:=Willem van den Akker <wvdakker@wilsoft.nl>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk

define Package/shorewall-lite
    SECTION:=net
    CATEGORY:=Network
    DEPENDS:=+ip +iptables +shorewall-core
    TITLE:=Shorewall Lite
    URL:=http://www.shorewall.net/
    SUBMENU:=Firewall
endef

define Package/shorewall-lite/description
	The Shoreline Firewall, is high-level tool for configuring Netfilter.

	Shorewall allows for central administration of multiple IPv4 firewalls
	through use of Shorewall lite. The full Shorewall product is installed
	on a central administrative system where compiled Shorewall scripts are
	generated. These scripts are copied to the firewall systems where they
	run under the control of Shorewall-lite.

	Note: This is the IPv4 implementation of Shorewall.
endef

define Package/shorewall-lite/conffiles
/etc/shorewall-lite/
endef

CONFIGURE_ARGS += \
	vendor=openwrt

define Build/Compile
	DESTDIR=$(PKG_INSTALL_DIR) $(PKG_BUILD_DIR)/install.sh
endef

define Package/shorewall-lite/install
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface/
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_DIR) $(1)/etc/shorewall-lite/state/
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/usr/share/shorewall-lite/
	$(CP) $(PKG_INSTALL_DIR)/etc/shorewall-lite/ $(1)/etc/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/shorewall-lite/ $(1)/usr/share/
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/shorewall-lite $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/shorewall-lite/shorecap $(1)/usr/share/shorewall-lite/
	$(INSTALL_BIN) ./files/hostname $(1)/usr/share/shorewall-lite/
	$(INSTALL_BIN) ./files/hotplug_iface $(1)/etc/hotplug.d/iface/05-shorewall-lite
	$(INSTALL_BIN) ./files/shorewall-lite.init $(1)/etc/init.d/shorewall-lite
	$(INSTALL_BIN) ./files/vardir $(1)/etc/shorewall-lite/
endef

$(eval $(call BuildPackage,shorewall-lite))
