#
# Copyright (C) 2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dante
PKG_VERSION:=1.4.1
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.inet.no/dante/files/
PKG_HASH:=b6d232bd6fefc87d14bf97e447e4fcdeef4b28b16b048d804b50b48f261c4f53

PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>
PKG_LICENSE:=BSD-4-Clause

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

CONFIGURE_ARGS += \
	--without-upnp \
	--without-pam \
	--disable-libwrap

CONFIGURE_VARS += \
	ac_cv_search_pam_start="" \
	ac_cv_func_sched_setscheduler=no

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.{a,so*,la} $(1)/usr/lib/
endef


define Package/dante/default
  TITLE:=Dante SOCKS
  URL:=http://www.inet.no/dante/
endef

define Package/dante/default/description
Dante is a circuit-level firewall/proxy that can be used to provide convenient
and secure network connectivity, requiring only that the server Dante runs on
has external network connectivity. Dante is used daily by Fortune 100 companies
and large international organizations, either as a standard SOCKS server or as
a "reverse proxy".
endef

define Package/libsocks
  $(call Package/dante/default)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE+= Library
  ABI_VERSION:=0
endef

define Package/libsocks/description
$(call Package/dante/default/description)
This package provides the shared libsocks library.
endef

define Package/libsocks/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libsocks.so.* $(1)/usr/lib/
endef


define Package/sockd
  $(call Package/dante/default)
  SUBMENU:=Web Servers/Proxies
  SECTION:=net
  CATEGORY:=Network
  TITLE+= Daemon
endef

define Package/sockd/description
$(call Package/dante/default/description)
This package provides the Dante sockd daemon.
endef

define Package/sockd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/sockd $(1)/usr/sbin/
endef


define Package/socksify
  $(call Package/dante/default)
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Web Servers/Proxies
  TITLE+= Client
endef

define Package/socksify/description
$(call Package/dante/default/description)
This package provides the Dante socksify client.
endef

define Package/socksify/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/socksify $(1)/usr/bin/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libdsocks.so* $(1)/usr/lib/
endef


$(eval $(call BuildPackage,libsocks))
$(eval $(call BuildPackage,sockd))
$(eval $(call BuildPackage,socksify))
