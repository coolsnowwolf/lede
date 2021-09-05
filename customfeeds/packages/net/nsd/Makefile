#
# Copyright (C) 2014-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=nsd
PKG_VERSION:=4.2.4
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://www.nlnetlabs.nl/downloads/nsd
PKG_HASH:=9ebd6d766765631a56c0eb332eac26b310fa39f662e5582c8210488cf91ef27c

PKG_MAINTAINER:=Vasilis Tsiligiannis <acinonyx@openwrt.gr>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/nsd/default
  TITLE:=NSD Name Server Daemon
  URL:=https://www.nlnetlabs.nl/projects/nsd/
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=IP Addresses and Names
endef

define Package/nsd/default/description
  NSD is an authoritative only, high performance, simple and open source name
  server.
endef

define Package/nsd
  $(Package/nsd/default)
  TITLE += (with SSL support)
  DEPENDS += +libopenssl
  VARIANT:=ssl
endef

define Package/nsd/description
  $(Package/nsd/default/description)
endef

define Package/nsd-nossl
  $(Package/nsd/default)
  TITLE += (without SSL support)
  VARIANT:=nossl
endef

define Package/nsd-nossl/description
  $(Package/nsd/default/description)
endef

define Package/nsd-checkconf/description
  $(Package/nsd/default/description)
endef

define Package/nsd-control
  $(Package/nsd/default)
  TITLE += remote control utility
  DEPENDS += +libopenssl
endef

define Package/nsd-control/description
  $(Package/nsd/default/description)
endef

define Package/nsd-control-setup
  $(Package/nsd/default)
  TITLE += SSL keys setup script
  DEPENDS += +openssl-util
endef

define Package/nsd-control-setup/description
  $(Package/nsd/default/description)
endef

CONFIGURE_ARGS+= \
	--enable-packed \
	--enable-recvmmsg \
	--enable-tcp-fastopen \
	--disable-checking \
	--with-user="network" \
	--without-libevent \
	$(if $(CONFIG_IPV6),--with,--without)-ipv6

CONFIGURE_VARS+= \
	ac_cv_c_va_list_def=no \
	ac_cv_c_strptime_needs_defs=no

ifeq ($(BUILD_VARIANT),ssl)
	CONFIGURE_ARGS += --with-ssl="$(STAGING_DIR)/usr"
else
	CONFIGURE_ARGS += --without-ssl
endif

define Package/nsd/conffiles
/etc/nsd/
endef

define Package/nsd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/{nsd,nsd-checkconf} $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/nsd
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/etc/nsd/nsd.conf.sample $(1)/etc/nsd/nsd.conf
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/nsd.init $(1)/etc/init.d/nsd
endef

define Package/nsd-control/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/nsd-control $(1)/usr/sbin/
endef

define Package/nsd-control-setup/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/nsd-control-setup $(1)/usr/sbin/
endef

Package/nsd-nossl/install = $(Package/nsd/install)
Package/nsd-nossl/conffiles = $(Package/nsd/conffiles)

$(eval $(call BuildPackage,nsd))
$(eval $(call BuildPackage,nsd-nossl))
$(eval $(call BuildPackage,nsd-control))
$(eval $(call BuildPackage,nsd-control-setup))
