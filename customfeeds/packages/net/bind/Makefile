#
# Copyright (C) 2006-2012 OpenWrt.org
#               2014-2020 Noah Meyerhans <frodo@morgul.net>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bind
PKG_VERSION:=9.17.13
PKG_RELEASE:=$(AUTORELEASE)
USERID:=bind=57:bind=57

PKG_MAINTAINER:=Noah Meyerhans <frodo@morgul.net>
PKG_LICENSE:=MPL-2.0
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:isc:bind

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:= \
	https://www.mirrorservice.org/sites/ftp.isc.org/isc/bind9/$(PKG_VERSION) \
	https://ftp.isc.org/isc/bind9/$(PKG_VERSION)
PKG_HASH:=bf485ac49715d43fa65c2c6e33271aab965bcd1b461fe2ac9f439754a210e6c7

PKG_FIXUP:=autoreconf
PKG_REMOVE_FILES:=aclocal.m4 libtool.m4

PKG_INSTALL:=1
PKG_USE_MIPS16:=0
PKG_BUILD_PARALLEL:=1

PKG_BUILD_DEPENDS:=nghttp2

PKG_CONFIG_DEPENDS := \
	CONFIG_BIND_LIBJSON \
	CONFIG_BIND_LIBXML2

PKG_BUILD_DEPENDS += BIND_LIBXML2:libxml2 BIND_LIBJSON:libjson-c

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/bind/Default
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+bind-libs +@OPENSSL_WITH_EC
  TITLE:=bind
  URL:=https://www.isc.org/software/bind
  SUBMENU:=IP Addresses and Names
endef

define Package/bind-libs
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libopenssl \
	+zlib \
	+libpthread \
	+libatomic \
	+libuv \
	+libnghttp2 \
	+BIND_LIBXML2:libxml2 \
	+BIND_LIBJSON:libjson-c
  TITLE:=bind shared libraries
  URL:=https://www.isc.org/software/bind
endef

define Package/bind-server
  $(call Package/bind/Default)
  TITLE+= DNS server
endef

define Package/bind-server/config
	source "$(SOURCE)/Config.in"
endef

define Package/bind-server-filter-aaaa
  $(call Package/bind-server)
  DEPENDS:=bind-server
  TITLE+= filter AAAA plugin
endef

define Package/bind-client
  $(call Package/bind/Default)
  TITLE+= dynamic DNS client
endef

define Package/bind-tools
  $(call Package/bind/Default)
  TITLE+= administration tools (all)
	DEPENDS:= \
	+bind-check \
	+bind-dig \
	+bind-nslookup \
	+bind-dnssec \
	+bind-host \
	+bind-rndc
endef

define Package/bind-rndc
  $(call Package/bind/Default)
  TITLE+= administration tools (rndc and rndc-confgen only)
endef

define Package/bind-check
  $(call Package/bind/Default)
  TITLE+= administration tools (named-checkconf and named-checkzone only)
endef

define Package/bind-dnssec
  $(call Package/bind/Default)
  TITLE+= administration tools (dnssec-keygen, dnssec-settime and dnssec-signzone only)
endef

define Package/bind-host
  $(call Package/bind/Default)
  TITLE+= simple DNS client
endef

define Package/bind-dig
  $(call Package/bind/Default)
  TITLE+= DNS excavation tool
endef

define Package/bind-nslookup
  $(call Package/bind/Default)
  TITLE+= nslookup utility
  ALTERNATIVES:= \
	  200:/usr/bin/nslookup:/usr/libexec/nslookup-bind
endef

export BUILD_CC="$(TARGET_CC)"

TARGET_LDFLAGS += -Wl,--gc-sections,--as-needed

CONFIGURE_ARGS += \
	--disable-linux-caps \
	--with-openssl="$(STAGING_DIR)/usr" \
	--with-libtool \
	--without-lmdb \
	--enable-epoll \
	--without-gssapi \
	--without-readline \
	--without-python \
	--sysconfdir=/etc/bind

ifdef CONFIG_BIND_LIBJSON
	TARGET_CFLAGS += -DHAVE_JSON_C -UHAVE_JSON
	CONFIGURE_ARGS += \
		--with-json-c=yes
else
	CONFIGURE_ARGS += \
		--with-json-c=no
endif

ifdef CONFIG_BIND_LIBXML2
	CONFIGURE_ARGS += \
		--with-libxml2=yes
else
	CONFIGURE_ARGS += \
		--with-libxml2=no
endif

CONFIGURE_VARS += \
	BUILD_CC="$(TARGET_CC)" \

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/lib/dns \
		BUILD_CC="$(HOSTCC)" \
		CC="$(HOSTCC)" \
		CFLAGS="-O2" \
		LIBS="" \
		gen
	$(call Build/Compile/Default)
endef

define Package/bind-libs/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib
endef

define Package/bind-server/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/named $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/bind
	$(CP) \
		./files/bind/db.0 \
		./files/bind/db.127 \
		./files/bind/db.255 \
		./files/bind/db.local \
		./files/bind/db.root \
		./files/bind/bind.keys \
		$(1)/etc/bind/
	sed -e '1s/ broadcast / empty rfc1918 /' \
		< ./files/bind/db.0 \
		> $(1)/etc/bind/db.empty
	$(CP) ./files/bind/named.conf.example $(1)/etc/bind/named.conf
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/named.init $(1)/etc/init.d/named
	find $(1)/etc/bind/ -name ".svn" | xargs rm -rf
endef

define Package/bind-server/conffiles
/etc/bind/db.0
/etc/bind/db.127
/etc/bind/db.255
/etc/bind/db.local
/etc/bind/db.root
/etc/bind/named.conf
endef

define Package/bind-server-filter-aaaa/install
	$(INSTALL_DIR) $(1)/usr/lib/bind
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/bind/filter-aaaa.so $(1)/usr/lib/bind
endef

define Package/bind-client/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/nsupdate $(1)/usr/bin/
endef

define Package/bind-tools/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/delv $(1)/usr/bin/
endef

define Package/bind-rndc/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/rndc $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/rndc-confgen $(1)/usr/sbin/
endef

define Package/bind-check/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/named-checkconf $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/named-checkzone $(1)/usr/bin/
endef

define Package/bind-dnssec/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/dnssec-keygen $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/dnssec-settime $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/dnssec-signzone $(1)/usr/bin/
endef

define Package/bind-host/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/host $(1)/usr/bin/
endef

define Package/bind-dig/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/dig $(1)/usr/bin/
endef

define Package/bind-nslookup/install
	$(INSTALL_DIR) $(1)/usr/libexec
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/nslookup $(1)/usr/libexec/nslookup-bind
endef

$(eval $(call BuildPackage,bind-libs))
$(eval $(call BuildPackage,bind-server))
$(eval $(call BuildPackage,bind-server-filter-aaaa))
$(eval $(call BuildPackage,bind-client))
$(eval $(call BuildPackage,bind-tools))
$(eval $(call BuildPackage,bind-rndc))
$(eval $(call BuildPackage,bind-check))
$(eval $(call BuildPackage,bind-dnssec))
$(eval $(call BuildPackage,bind-host))
$(eval $(call BuildPackage,bind-dig))
$(eval $(call BuildPackage,bind-nslookup))
