#
# Copyright (C) 2010 Christian Pointner,
#                    <equinox@spreadspace.org>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# This Makefile builds tcpproxy Package for OpenWRT
#
# $Id: $

include $(TOPDIR)/rules.mk

PKG_NAME:=tcpproxy
PKG_VERSION:=1.2
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.spreadspace.org/tcpproxy/releases/
PKG_HASH:=446104b3abefda73b1255e5197702acbeb36b3aada251ad39e1e5ab55292683b
PKG_MAINTAINER:=Christian Pointner <equinox@spreadspace.org>
PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=LICENSE
include $(INCLUDE_DIR)/package.mk


define Package/tcpproxy
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Routing and Redirection
  TITLE:=IPv4/IPv6 tcp connection proxy
  URL:=http://www.spreadspace.org/tcpproxy/
endef

define Package/tcpproxy/conffiles
/etc/config/tcpproxy
endef

define Package/tcpproxy/description
tcpproxy is a simple tcp connection proxy which combines the features of rinetd and 6tunnel.
 tcpproxy supports IPv4 and IPv6 and also supports connections from IPv6 to IPv4 endpoints and vice versa.
endef

define Package/tcpproxy/configure
endef

define Build/Configure
	(cd $(PKG_BUILD_DIR)/src; \
    touch include.mk; \
    echo '#ifndef TCPPROXY_config_h_INCLUDED' > config.h; \
    echo '#define TCPPROXY_config_h_INCLUDED' >> config.h; \
    echo '' >> config.h; \
    echo '#define VERSION_STRING_0 "tcpproxy version '`cat $(PKG_BUILD_DIR)/version`'"' >> config.h; \
    echo '' >> config.h; \
    echo '#define TARGET "linux"' >> config.h; \
    echo '#define PREFIX "/usr"' >> config.h; \
    echo '#define BINDIR "/usr/bin"' >> config.h; \
    echo '#define SYSTEMDDIR "/tmp"' >> config.h; \
    echo '#define ETCDIR "/etc"' >> config.h; \
    echo '#define CONFFILE "/etc/tcpproxy.conf"' >> config.h; \
    echo '' >> config.h; \
    echo '#endif' >> config.h \
  )
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
    $(TARGET_CONFIGURE_OPTS) \
    TARGET=Linux \
    CFLAGS="$(TARGET_CFLAGS)" \
    LDFLAGS="$(TARGET_LDFLAGS)"
	$(STRIP) $(PKG_BUILD_DIR)/src/tcpproxy
endef

define Package/tcpproxy/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/tcpproxy.config $(1)/etc/config/tcpproxy
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(2)/src/tcpproxy $(1)/usr/bin/tcpproxy
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/tcpproxy.init $(1)/etc/init.d/tcpproxy
endef

$(eval $(call BuildPackage,tcpproxy))
