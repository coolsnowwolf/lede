#
# Copyright (C) 2008-2014 Christian Pointner,
#                         <equinox@anytun.org>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# This Makefile builds uAnytun Package for OpenWRT
#
# $Id: $

include $(TOPDIR)/rules.mk

PKG_NAME:=uanytun
PKG_VERSION:=0.3.7
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.anytun.org/download/
PKG_HASH:=076318c771c908386c4408dda0769171542da701785302903641ce23fd33a295
PKG_MAINTAINER:=Christian Pointner <equinox@spreadspace.org>
PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=LICENSE
include $(INCLUDE_DIR)/package.mk


define Package/uanytun/template
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=VPN
  DEPENDS:=+kmod-tun
  TITLE:=micro anycast tunneling daemon
  URL:=http://www.anytun.org/
endef


define Package/uanytun
  $(call Package/uanytun/template)
  TITLE+= (gcrypt)
  VARIANT:=gcrypt
  DEPENDS+=+libgcrypt
endef

define Package/uanytun/conffiles
/etc/config/uanytun
endef

define Package/uanytun/description
uAnytun is a tiny implementation of SATP the secure anycast tunneling protocol.
  SATP defines a protocol used for communication between any combination of
  unicast and anycast tunnel endpoints.  It has less protocol overhead than
  IPSec in Tunnel mode and allows tunneling of every ETHER TYPE protocol (e.g.
  ethernet, ip, arp ...). SATP directly includes cryptography and message
  authentication based on the methods used by SRTP.  It is intended to deliver
  a generic, scalable and secure solution for tunneling and relaying of packets
  of any protocol.
  Unlike Anytun which is a full featured implementation uAnytun has no support
  for multiple connections or synchronisation. It is a small single threaded
  implementation intended to act as a client on small platforms.
endef


define Package/uanytun-nettle
  $(call Package/uanytun/template)
  TITLE+= (nettle)
  VARIANT:=nettle
  DEPENDS+=+libnettle
endef

Package/uanytun-nettle/conffiles=$(Package/uanytun/conffiles)
Package/uanytun-nettle/description=$(Package/uanytun/description)


define Package/uanytun-sslcrypt
  $(call Package/uanytun/template)
  TITLE+= (openssl)
  VARIANT:=sslcrypt
  DEPENDS+=+libopenssl
endef

Package/uanytun-sslcrypt/conffiles=$(Package/uanytun/conffiles)
Package/uanytun-sslcrypt/description=$(Package/uanytun/description)


define Package/uanytun-nocrypt
  $(call Package/uanytun/template)
  TITLE+= (no crypt)
  VARIANT:=nocrypt
endef

Package/uanytun-nocrypt/conffiles=$(Package/uanytun/conffiles)
Package/uanytun-nocrypt/description=$(Package/uanytun/description)



define Build/Configure
	(cd $(PKG_BUILD_DIR)/src; \
    touch include.mk; \
    ln -s linux/tun.c .; \
    echo '#ifndef UANYTUN_version_h_INCLUDED' > version.h; \
    echo '#define UANYTUN_version_h_INCLUDED' >> version.h; \
    echo '' >> version.h; \
    echo '#define VERSION_STRING "uanytun version '`cat $(PKG_BUILD_DIR)/version`'"' >> version.h; \
    echo '' >> version.h; \
    echo '#endif' >> version.h \
  )
endef

VARIANT_CFLAGS:=
VARIANT_LDFLAGS:=-ldl
VARIANT_MAKE_OPTS:=

ifeq ($(BUILD_VARIANT),gcrypt)
VARIANT_CFLAGS+=-DUSE_GCRYPT -DCRYPTO_LIB_NAME=\\\"libgcrypt\\\"
VARIANT_LDFLAGS+=-lgpg-error -lgcrypt
endif

ifeq ($(BUILD_VARIANT),nettle)
VARIANT_CFLAGS+=-DUSE_NETTLE -DCRYPTO_LIB_NAME=\\\"Nettle\\\"
VARIANT_LDFLAGS+=-lnettle
endif

ifeq ($(BUILD_VARIANT),sslcrypt)
VARIANT_CFLAGS+=-DUSE_SSL_CRYPTO -DCRYPTO_LIB_NAME=\\\"OpenSSL\\\"
VARIANT_LDFLAGS+=-lcrypto
endif

ifeq ($(BUILD_VARIANT),nocrypt)
VARIANT_CFLAGS+=-DNO_CRYPT -DCRYPTO_LIB_NAME=\\\"none\\\"
VARIANT_MAKE_OPTS+=NO_CRYPT_OBJ=1
endif

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
    $(TARGET_CONFIGURE_OPTS) \
    $(VARIANT_MAKE_OPTS) \
    TARGET=Linux \
    CFLAGS="$(TARGET_CFLAGS) $(VARIANT_CFLAGS) $(TARGET_CPPFLAGS)" \
    LDFLAGS="$(TARGET_LDFLAGS) $(VARIANT_LDFLAGS)"
	$(STRIP) $(PKG_BUILD_DIR)/src/uanytun
endef


define Package/uanytun/install-generic
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/$(2) $(1)/etc/config/$(PKG_NAME)
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/uanytun $(1)/usr/sbin/$(PKG_NAME)
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/uanytun.init $(1)/etc/init.d/$(PKG_NAME)
endef

define Package/uanytun/install
  $(call Package/uanytun/install-generic,$(1),uanytun.config)
endef

define Package/uanytun-nettle/install
  $(call Package/uanytun/install-generic,$(1),uanytun.config)
endef

define Package/uanytun-sslcrypt/install
  $(call Package/uanytun/install-generic,$(1),uanytun.config)
endef

define Package/uanytun-nocrypt/install
  $(call Package/uanytun/install-generic,$(1),uanytun-nocrypt.config)
endef


$(eval $(call BuildPackage,uanytun))
$(eval $(call BuildPackage,uanytun-nettle))
$(eval $(call BuildPackage,uanytun-sslcrypt))
$(eval $(call BuildPackage,uanytun-nocrypt))
