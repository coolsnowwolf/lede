#
# Copyright (C) 2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dhcp-forwarder
PKG_VERSION:=0.11
PKG_RELEASE:=1

PKG_SOURCE_URL:=@SAVANNAH/dhcp-fwd/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=6abf8e6a72ad01df90efba543c9a18c149b24d50d000e66ce55209780dc04cd5

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING

PKG_MAINTAINER:=Scott K Logan <logans@cottsay.net>

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/dhcp-forwarder
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=IP Addresses and Names
  TITLE:=DHCP relay agent
  URL:=http://www.nongnu.org/dhcp-fwd/
  USERID:=dhcp-fwd=375:dhcp-fwd=375
endef

define Package/dhcp-forwarder/description
  This program forwards DHCP messages between subnets with different sublayer
  broadcast domains. It is similarly to the DHCP relay agent dhcrelay of ISC's
  DHCP, but has the following features which are important for me:

  - Runs as non-root in a chroot-jail
  - Uses "normal" AF_INET sockets; this allows to uses packetfilters to filter
    incoming messages.
  - The DHCP agent IDs can be defined freely
  - Has a small memory footprint when using dietlibc

  It tries to be RFC 2131/2132 and 3046 compliant. Probably it works with BOOTP
  (RFC 951) also, but since the handling of the giaddr field is described
  misleading (see RFC 1542), there may occur problems.
endef

define Package/dhcp-forwarder/conffiles
/etc/dhcp-fwd.conf
endef

CONFIGURE_ARGS += \
	--disable-dietlibc \

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
	  DESTDIR="$(PKG_INSTALL_DIR)" \
	  cfg_filename="/etc/dhcp-fwd.conf" \
	  all install
endef

define Package/dhcp-forwarder/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/etc $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/dhcp-fwd $(1)/usr/bin/
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/contrib/dhcp-fwd.conf $(1)/etc/
	$(INSTALL_BIN) ./files/dhcp-fwd.init $(1)/etc/init.d/dhcp-fwd
endef

$(eval $(call BuildPackage,dhcp-forwarder))
