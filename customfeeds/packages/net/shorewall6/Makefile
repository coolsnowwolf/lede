#
# Copyright (C) 2008-2012 OpenWrt.org
# Copyright (C) 2017 Willem van den Akker <wvdakker@wilsoft.nl>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=shorewall6
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
PKG_HASH:=e82c3a9538b6548195398dc39c83b01bcf66eb5a22814c915a924b9adc088cd1

PKG_MAINTAINER:=Willem van den Akker <wvdakker@wilsoft.nl>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk

define Package/shorewall6
    SECTION:=net
    CATEGORY:=Network
    DEPENDS:=+ip +ip6tables +kmod-ipt-hashlimit +kmod-ipt-raw6 +iptables-mod-hashlimit +shorewall-core \
		+perl +perlbase-autoloader +perlbase-autouse +perlbase-dynaloader +perlbase-digest \
		+perlbase-findbin +perlbase-getopt +perlbase-hash
    TITLE:=Shorewall6 Central Administration System
    URL:=http://www.shorewall.net/
    SUBMENU:=Firewall
endef

define Package/shorewall6/description
	The Shoreline Firewall, is high-level tool for configuring Netfilter.

	Shorewall allows for central administration of multiple IPv6 firewalls.
	This is the full Shorewall product which will compile Shorewall scripts
	It is not recommended to run it on a low memory system.

	Note: This is the IPv6 implementation of Shorewall.
	      This full Shorewal packages also installs Perl which can make the image big (about +2M).
endef

CONFIGURE_ARGS += \
	vendor=openwrt

define Package/shorewall6/conffiles
/etc/shorewall6/
endef

define Build/Compile
	DESTDIR=$(PKG_INSTALL_DIR) $(PKG_BUILD_DIR)/install.sh
endef

define Package/shorewall6/install
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_DIR) $(1)/etc/shorewall6/
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/usr/share/shorewall6/
	$(INSTALL_BIN) ./files/hostname $(1)/etc/shorewall6/
	$(INSTALL_BIN) ./files/hotplug_iface $(1)/etc/hotplug.d/iface/05-shorewall6
	$(INSTALL_BIN) ./files/shorewall6.init $(1)/etc/init.d/shorewall6
	$(INSTALL_BIN) ./files/vardir $(1)/etc/shorewall6/
	$(CP) $(PKG_INSTALL_DIR)/etc/shorewall6/. $(1)/etc/shorewall6/
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/shorewall6 $(1)/usr/sbin/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/shorewall6/. $(1)/usr/share/shorewall6/
endef

$(eval $(call BuildPackage,shorewall6))
