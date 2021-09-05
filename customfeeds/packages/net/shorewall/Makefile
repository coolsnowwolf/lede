#
# Copyright (C) 2008-2012 OpenWrt.org
# Copyright (C) 2017 Willem van den Akker <wvdakker@wilsoft.nl>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=shorewall
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
PKG_HASH:=fbb5ab492ed372eaaf02817cc730f82c49a81c5a5f5cee4bc8f1bce846f2306d

PKG_MAINTAINER:=Willem van den Akker <wvdakker@wilsoft.nl>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk

define Package/shorewall
    SECTION:=devel
    CATEGORY:=Network
    DEPENDS:=+ip +iptables +kmod-ipt-hashlimit +kmod-ipt-raw +iptables-mod-hashlimit +shorewall-core \
		+perl +perlbase-autoloader +perlbase-autouse +perlbase-dynaloader +perlbase-digest \
		+perlbase-findbin +perlbase-getopt +perlbase-hash
    TITLE:=Shorewall Central Administration System
    URL:=http://www.shorewall.net/
    SUBMENU:=Firewall
endef

define Package/shorewall/description
	The Shoreline Firewall, is high-level tool for configuring Netfilter.

	Shorewall allows for central administration of multiple IPv4 firewalls.
	This is the full Shorewall product which will compile Shorewall scripts.
	It is not recommended to run it on a low memory system.

	Note: This is the IPv4 implementation of Shorewall.
	      This full Shorewal packages also installs Perl which can make the image big (about +2M).
endef

CONFIGURE_ARGS += \
	vendor=openwrt

define Build/Compile
	DESTDIR=$(PKG_INSTALL_DIR) $(PKG_BUILD_DIR)/install.sh
endef

define Package/shorewall/conffiles
/etc/shorewall/
endef

define Package/shorewall/install
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface/
	$(INSTALL_DIR) $(1)/etc/shorewall/
	$(INSTALL_DIR) $(1)/usr/lib/shorewall/
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/usr/share/shorewall/
	$(INSTALL_BIN) ./files/hotplug_iface $(1)/etc/hotplug.d/iface/05-shorewall
	$(INSTALL_BIN) ./files/hostname $(1)/etc/shorewall/
	$(INSTALL_BIN) ./files/shorewall.init $(1)/etc/init.d/shorewall
	$(INSTALL_BIN) ./files/vardir $(1)/etc/shorewall/
	$(CP) $(PKG_INSTALL_DIR)/etc/shorewall/. $(1)/etc/shorewall/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/shorewall/. $(1)/usr/lib/shorewall/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/shorewall/. $(1)/usr/share/shorewall/
endef

$(eval $(call BuildPackage,shorewall))
