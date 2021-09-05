#
# Copyright (C) 2010-2016 OpenWrt.org
# Copyright (C) 2009-2016 Thomas Heil <heil@terminal-consulting.de>
# Copyright (C) 2018 Christian Lachner <gladiac@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=haproxy
PKG_VERSION:=2.4.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://www.haproxy.org/download/2.4/src
PKG_HASH:=edf9788f7f3411498e3d7b21777036b4dc14183e95c8e2ce7577baa0ea4ea2aa

PKG_MAINTAINER:=Thomas Heil <heil@terminal-consulting.de>, \
		Christian Lachner <gladiac@gmail.com>
PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:haproxy:haproxy

include $(INCLUDE_DIR)/package.mk

define Package/haproxy/Default
  SUBMENU:=Web Servers/Proxies
  SECTION:=net
  CATEGORY:=Network
  TITLE:=The Reliable, High Performance TCP/HTTP Load Balancer
  URL:=https://www.haproxy.org/
endef

define Build/Prepare
	$(call Build/Prepare/Default)
endef

define Package/haproxy/Default/conffiles
/etc/haproxy.cfg
endef

define Package/haproxy/Default/description
 Open source Reliable, High Performance TCP/HTTP Load Balancer.
endef

define Package/haproxy
  DEPENDS+= +libpcre +libltdl +zlib +libpthread +liblua5.3 +libopenssl +libncurses +libreadline +libatomic
  TITLE+= (with SSL support)
  VARIANT:=ssl
$(call Package/haproxy/Default)
endef

define Package/haproxy/conffiles
$(call Package/haproxy/Default/conffiles)
endef

define Package/haproxy/description
$(call Package/haproxy/Default/description)
 This package is built with SSL and LUA support.
endef

define Package/haproxy-nossl
  TITLE+= (without SSL support)
  VARIANT:=nossl
  DEPENDS+= +libpcre +libltdl +zlib +libpthread +liblua5.3 +libatomic
  TITLE+= (without SSL support)
$(call Package/haproxy/Default)
endef

define Package/haproxy-nossl/conffiles
$(call Package/haproxy/Default/conffiles)
endef

define Package/haproxy-nossl/description
$(call Package/haproxy/Default/description)
 This package is built without SSL support.
endef

ENABLE_LUA:=y

ifeq ($(CONFIG_USE_UCLIBC),y)
	ADDON+=USE_LIBCRYPT=
endif

ifeq ($(BUILD_VARIANT),ssl)
	ADDON+=USE_OPENSSL=1
	ADDON+=ADDLIB="-lcrypto -lm"
endif

define Build/Compile
	$(MAKE) TARGET=linux-glibc -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		CC="$(TARGET_CC)" \
		PCREDIR="$(STAGING_DIR)/usr/" \
		USE_LUA=1 LUA_LIB_NAME="lua5.3" LUA_INC="$(STAGING_DIR)/usr/include/lua5.3" LUA_LIB="$(STAGING_DIR)/usr/lib" \
		SMALL_OPTS="-DBUFSIZE=16384 -DMAXREWRITE=1030 -DSYSTEM_MAXCONN=165530" \
		USE_LINUX_TPROXY=1 USE_LINUX_SPLICE=1 USE_TFO=1 USE_NS=1 \
		USE_ZLIB=1 USE_PCRE=1 USE_PCRE_JIT=1 USE_GETADDRINFO=1 \
		USE_THREAD=1 USE_PTHREAD_PSHARED=1 USE_PROMEX=1 \
		VERSION="$(PKG_VERSION)" SUBVERS="-$(PKG_RELEASE)" \
		VERDATE="$(shell date -d @$(SOURCE_DATE_EPOCH) '+%Y/%m/%d')" IGNOREGIT=1 \
		$(ADDON) USE_BACKTRACE= \
		CFLAGS="$(TARGET_CFLAGS) -fno-strict-aliasing -Wdeclaration-after-statement -Wno-unused-label -Wno-sign-compare -Wno-unused-parameter -Wno-clobbered -Wno-missing-field-initializers -Wno-cast-function-type -Wtype-limits -Wshift-negative-value -Wshift-overflow=2 -Wduplicated-cond -Wnull-dereference -fwrapv -fasynchronous-unwind-tables -Wno-null-dereference" \
		LD="$(TARGET_CC)" \
		LDFLAGS="$(TARGET_LDFLAGS) -latomic"

	$(MAKE_VARS) $(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		LD="$(TARGET_CC)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		$(MAKE_FLAGS) \
		install

	$(MAKE_VARS) $(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		admin/halog/halog
endef

define Package/haproxy/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/haproxy $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_CONF) ./files/haproxy.cfg $(1)/etc/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/haproxy.init $(1)/etc/init.d/haproxy
endef

Package/haproxy-nossl/install = $(Package/haproxy/install)

define Package/halog
	MENU:=1
	$(call Package/haproxy)
	TITLE+= halog
	DEPENDS:=haproxy
endef

define Package/halog/description
	HAProxy Log Analyzer
endef

define Package/halog/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/admin/halog/halog $(1)/usr/bin/
endef

$(eval $(call BuildPackage,haproxy))
$(eval $(call BuildPackage,halog))
$(eval $(call BuildPackage,haproxy-nossl))
