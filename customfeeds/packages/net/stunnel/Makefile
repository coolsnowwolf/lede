#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=stunnel
PKG_VERSION:=5.59
PKG_RELEASE:=1

PKG_LICENSE:=GPL-2.0-or-later
PKG_MAINTAINER:=Florian Eckert <fe@dev.tdt.de>
PKG_LICENSE_FILES:=COPYING.md COPYRIGHT.md
PKG_CPE_ID:=cpe:/a:stunnel:stunnel

PKG_SOURCE_URL:= \
	http://ftp.nluug.nl/pub/networking/stunnel/ \
	http://www.usenix.org.uk/mirrors/stunnel/ \
	https://www.stunnel.org/downloads/ \
	https://www.usenix.org.uk/mirrors/stunnel/archive/$(word 1, $(subst .,$(space),$(PKG_VERSION))).x/

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=137776df6be8f1701f1cd590b7779932e123479fb91e5192171c16798815ce9f

PKG_FIXUP:=autoreconf
PKG_FIXUP:=patch-libtool
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/stunnel
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libopenssl +zlib +libpthread
  TITLE:=SSL TCP Wrapper
  URL:=http://www.stunnel.org/
endef

define Package/stunnel/description
	Stunnel is a program that allows you to encrypt arbitrary TCP
	connections inside SSL (Secure Sockets Layer) available on both Unix
	and Windows. Stunnel can allow you to secure non-SSL aware daemons and
	protocols (like POP, IMAP, LDAP, etc) by having Stunnel provide the
	encryption, requiring no changes to the daemon's code.
endef

define Package/stunnel/conffiles
/etc/stunnel/stunnel.conf
/etc/config/stunnel
endef


CONFIGURE_ARGS+= \
	--with-random=/dev/urandom \
	--with-threads=pthread \
	--with-ssl=$(STAGING_DIR)/usr \
	--disable-libwrap \
	--disable-systemd

ifeq ($(CONFIG_IPV6),n)
CONFIGURE_ARGS+= \
	--disable-ipv6
endif

define Build/Compile
	mkdir -p $(PKG_INSTALL_DIR)/etc/stunnel
	echo '#dummy' > $(PKG_INSTALL_DIR)/etc/stunnel/stunnel.pem
	$(call Build/Compile/Default)
endef

define Package/stunnel/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/stunnel $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib/stunnel
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/stunnel/libstunnel.so $(1)/usr/lib/stunnel/
	$(INSTALL_DIR) $(1)/etc/stunnel
	$(INSTALL_CONF) ./files/stunnel.conf $(1)/etc/stunnel/stunnel.conf
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/stunnel.init $(1)/etc/init.d/stunnel
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/stunnel.uci $(1)/etc/config/stunnel
endef

$(eval $(call BuildPackage,stunnel))
