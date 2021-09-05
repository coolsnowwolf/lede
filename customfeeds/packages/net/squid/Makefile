#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=squid
PKG_VERSION:=4.15
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=http://www2.pl.squid-cache.org/Versions/v4/ \
	http://www.squid-cache.org/Versions/v4/
PKG_HASH:=b693a4e5ab2811a8a854f60de0a62afbbf3a952bb1d047952c9ae01321f84a25

PKG_MAINTAINER:=Marko Ratkaj <marko.ratkaj@sartura.hr>
PKG_LICENSE:=GPL-2.0-or-later
PKG_CPE_ID:=cpe:/a:squid-cache:squid

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/squid/Default
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Web Servers/Proxies
  URL:=http://www.squid-cache.org/
endef

define Package/squid
  $(call Package/squid/Default)
  MENU:=1
  DEPENDS:=+libpthread +librt +libltdl +libstdcpp +libatomic +USE_GLIBC:libbsd
  DEPENDS+= +SQUID_use-gnutls:libgnutls +SQUID_use-openssl:libopenssl
  DEPENDS+= +SQUID_with-libcap:libcap
  DEPENDS+= +SQUID_with-nettle:libnettle
  DEPENDS+= +SQUID_with-expat:libexpat
  DEPENDS+= +SQUID_with-libxml2:libxml2
  USERID:=squid=137:squid=137
  TITLE:=full-featured Web proxy cache
endef

define Package/squid/description
  Squid is a caching proxy for the Web supporting HTTP, HTTPS, FTP, and more.
  It reduces bandwidth and improves response times by caching and reusing
  frequently-requested web pages.
endef

define Package/squid/config
  source "$(SOURCE)/Config.in"
endef

define Package/squid/conffiles
/etc/config/squid
/etc/squid/squid.conf
endef

define Package/squid-mod-cachemgr
  $(call Package/squid/Default)
  DEPENDS:=squid
  TITLE:=Web based proxy manager and reporting tool
endef

CONFIGURE_ARGS += \
	BUILDCXX=$(HOSTCXX_NOCACHE) \
	--config-cache \
	--datadir=/usr/share/squid \
	--libexecdir=/usr/lib/squid \
	--sysconfdir=/etc/squid \
	--enable-shared \
	--disable-static \
	--enable-delay-pools \
	--enable-kill-parent-hack \
	--enable-ssl \
	--enable-cache-digests \
	--enable-linux-netfilter \
	--disable-unlinkd \
	--enable-x-accelerator-vary \
	--disable-translation \
	--disable-auto-locale \
	--with-dl \
	--with-pthreads \
	--enable-epoll \
	--with-maxfd=2048 \
	--disable-ecap \
	--disable-external-acl-helpers \
	--disable-arch-native \
	--with-krb5-config=no \
	--without-mit-krb5 \
	--without-netfilter-conntrack \
	--disable-ident-lookups \
	$(if $(CONFIG_SQUID_auth-basic),--enable,--disable)-auth-basic \
	$(if $(CONFIG_SQUID_auth-digest),--enable,--disable)-auth-digest \
	$(if $(CONFIG_SQUID_auth-ntlm),--enable,--disable)-auth-ntlm \
	$(if $(CONFIG_SQUID_auth-negotiate),--enable,--disable)-auth-negotiate \
	$(if $(CONFIG_SQUID_enable-ipv6),--enable,--disable)-dlmalloc \
	$(if $(CONFIG_SQUID_enable-ipv6),--enable,--disable)-ipv6 \
	$(if $(CONFIG_SQUID_enable-ssl-crtd),--enable-ssl-crtd) \
	$(if $(CONFIG_SQUID_use-gnutls),--with,--without)-gnutls \
	$(if $(CONFIG_SQUID_use-gnutls),--without-openssl) \
	$(if $(CONFIG_SQUID_use-gnutls),,--with-openssl="$(STAGING_DIR)/usr") \
	$(if $(CONFIG_SQUID_enable-icmp),--enable,--disable)-icmp \
	$(if $(CONFIG_SQUID_enable-icap-client),--enable,--disable)-icap-client \
	$(if $(CONFIG_SQUID_enable-snmp),--enable,--disable)-snmp \
	$(if $(CONFIG_SQUID_with-libcap),--with,--without)-libcap \
	$(if $(CONFIG_SQUID_with-nettle),--with,--without)-nettle \
	$(if $(CONFIG_SQUID_with-expat),--with,--without)-expat \
	$(if $(CONFIG_SQUID_with-libxml2),--with,--without)-libxml2

CONFIGURE_VARS += \
	ac_cv_header_linux_netfilter_ipv4_h=yes \
	ac_cv_epoll_works=yes

TARGET_CFLAGS += -Wno-error
TARGET_LDFLAGS += -latomic

define Package/squid/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/squid $(1)/usr/sbin/

	$(INSTALL_DIR) $(1)/usr/lib/squid
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/squid/* $(1)/usr/lib/squid/

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/squid.config $(1)/etc/config/squid

	$(INSTALL_DIR) $(1)/etc/squid
	$(CP) $(PKG_INSTALL_DIR)/etc/squid/* $(1)/etc/squid/
	$(INSTALL_CONF) ./files/squid.conf $(1)/etc/squid/

	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/squid.init $(1)/etc/init.d/squid

	$(INSTALL_DIR) $(1)/usr/share/squid/icons/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/squid/icons/* $(1)/usr/share/squid/icons/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/squid/mib.txt $(1)/usr/share/squid/

	$(INSTALL_DIR) $(1)/usr/share/squid/errors/templates/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/squid/errors/templates/* $(1)/usr/share/squid/errors/templates/
endef

define Package/squid-mod-cachemgr/install
	$(INSTALL_DIR) $(1)/www/cgi-bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/squid/cachemgr.cgi $(1)/www/cgi-bin/
endef

$(eval $(call BuildPackage,squid))
$(eval $(call BuildPackage,squid-mod-cachemgr))
