#
# Copyright (C) 2008-2012 OpenWrt.org
# Copyright (C) 2017 Willem van den Akker <wvdakker@wilsoft.nl>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=shorewall6-lite
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
PKG_HASH:=086764bb976d199b45563e39b1ab9a131ccb521d60d186f606dd8bbcee7fcada

PKG_MAINTAINER:=Willem van den Akker <wvdakker@wilsoft.nl>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING


include $(INCLUDE_DIR)/package.mk

define Package/shorewall6-lite
    SECTION:=net
    CATEGORY:=Network
    DEPENDS:=+ip +ip6tables +shorewall-core
    TITLE:=Shorewall6 Lite
    URL:=http://www.shorewall.net/
    SUBMENU:=Firewall
endef

define Package/shorewall6-lite/description
	The Shoreline Firewall, is high-level tool for configuring Netfilter.

	Shorewall6 allows for central administration of multiple IPv6 firewalls
	through use of Shorewall6 lite. The full Shorewall6 product is installed
	on a central administrative system where compiled Shorewall6 scripts are
	generated. These scripts are copied to the firewall systems where they
	run under the control of Shorewall6-lite.

	Note: This is the IPv6 implementation of Shorewall.
endef

define Package/shorewall6-lite/conffiles
/etc/shorewall6-lite/
endef

CONFIGURE_ARGS += \
	vendor=openwrt

define Build/Compile
	DESTDIR=$(PKG_INSTALL_DIR) $(PKG_BUILD_DIR)/install.sh
endef

define Package/shorewall6-lite/install
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface/
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_DIR) $(1)/etc/shorewall6-lite/state/
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/usr/share/shorewall6-lite/
	$(CP) $(PKG_INSTALL_DIR)/etc/shorewall6-lite/ $(1)/etc/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/shorewall6-lite/ $(1)/usr/share/
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/shorewall6-lite $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/shorewall6-lite/shorecap $(1)/usr/share/shorewall6-lite/
	$(INSTALL_BIN) ./files/hostname $(1)/usr/share/shorewall6-lite/
	$(INSTALL_BIN) ./files/hotplug_iface $(1)/etc/hotplug.d/iface/05-shorewall6-lite
	$(INSTALL_BIN) ./files/shorewall6-lite.init $(1)/etc/init.d/shorewall6-lite
	$(INSTALL_BIN) ./files/vardir $(1)/etc/shorewall6-lite/
endef

$(eval $(call BuildPackage,shorewall6-lite))
