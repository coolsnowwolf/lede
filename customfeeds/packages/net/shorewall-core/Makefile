#
# Copyright (C) 2008-2012 OpenWrt.org
# Copyright (C) 2017 Willem van den Akker <wvdakker@wilsoft.nl>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=shorewall-core
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
PKG_HASH:=39996b9697a2017947049ad3f03cb278e8f939ff924b2bb4bf2a0bc17c59988e

PKG_MAINTAINER:=Willem van den Akker <wvdakker@wilsoft.nl>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk

define Package/shorewall-core
    SECTION:=net
    CATEGORY:=Network
    TITLE:=Shorewall Core
    URL:=http://www.shorewall.net/
    SUBMENU:=Firewall
endef

define Package/shorewall-core/description
	The Shoreline Firewall, is high-level tool for configuring Netfilter.
	This package provides the core Shorewall libraries installed in /usr/share/shorewall/,
	which are required for the rest of the Shorewall packages to work.
endef

define Package/shorewall-core/conffiles
/usr/share/shorewall/shorewallrc
endef

CONFIGURE_ARGS += \
	vendor=openwrt

define Build/Compile
	DESTDIR=$(PKG_INSTALL_DIR) $(PKG_BUILD_DIR)/install.sh
endef

define Package/shorewall-core/install
	$(INSTALL_DIR) $(1)/usr/share/shorewall/
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/shorewall $(1)/usr/share/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/shorewall/wait4ifup $(1)/usr/share/shorewall/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/shorewall $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,shorewall-core))
