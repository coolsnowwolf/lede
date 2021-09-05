#
# Copyright (C) 2010-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=unbound
PKG_VERSION:=1.13.1
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://nlnetlabs.nl/downloads/unbound
PKG_HASH:=8504d97b8fc5bd897345c95d116e0ee0ddf8c8ff99590ab2b4bd13278c9f50b8

PKG_MAINTAINER:=Eric Luehrsen <ericluehrsen@gmail.com>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:nlnetlabs:unbound

PKG_BUILD_PARALLEL:=1
PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

PKG_CONFIG_DEPENDS:=CONFIG_PACKAGE_libunbound_dnscrypt \
	CONFIG_PACKAGE_libunbound_ipset \
	CONFIG_PACKAGE_libunbound_libevent \
	CONFIG_PACKAGE_libunbound_libpthread \
	CONFIG_PACKAGE_libunbound_nghttp2 \
	CONFIG_PACKAGE_libunbound_pythonmodule \
	CONFIG_PACKAGE_libunbound_subnet \
	CONFIG_PACKAGE_libunbound_dnstap

include $(INCLUDE_DIR)/package.mk

define Package/unbound/Default
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=IP Addresses and Names
  USERID:=unbound:unbound
  TITLE:=Recursive DNS Server
  URL:=https://nlnetlabs.nl/projects/unbound/about
  DEPENDS:=+ca-bundle +libopenssl +@OPENSSL_WITH_EC
endef

define Package/unbound-daemon
  $(call Package/unbound/Default)
  TITLE+= (daemon)
  DEPENDS+= +libunbound
endef

define Package/unbound-daemon/description
  This package contains the Unbound daemon including 'libevent', 'libmnl', and
  'libpthread' to better handle large networks with heavy query loads. Options
  are available under libraries/network/libunbound to custom trim Unbound for
  smaller targets.
endef

define Package/libunbound
  $(call Package/unbound/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  SUBMENU:=Networking
  TITLE+= (library)
  DEPENDS+=+PACKAGE_libunbound_dnscrypt:libsodium \
    +PACKAGE_libunbound_ipset:libmnl \
    +PACKAGE_libunbound_libevent:libevent2 \
    +PACKAGE_libunbound_libpthread:libpthread \
    +PACKAGE_libunbound_nghttp2:libnghttp2 \
    +PACKAGE_libunbound_pythonmodule:python3-base \
    +PACKAGE_libunbound_dnstap:libprotobuf-c
endef

define Package/libunbound/description
  This package contains the Unbound library including 'libevent', 'libmnl', and
  'libpthread' to better handle large networks with heavy query loads. Options
  are available to custom trim Unbound for smaller targets.
endef

define Package/unbound-anchor
  $(call Package/unbound/Default)
  TITLE+= (root DSKEY)
  DEPENDS+= +unbound-daemon +libexpat
endef

define Package/unbound-anchor/description
  This package contains the Unbound anchor utility.
endef

define Package/unbound-checkconf
  $(call Package/unbound/Default)
  TITLE+= (config checker)
  DEPENDS+= +unbound-daemon
endef

define Package/unbound-checkconf/description
  This package contains the Unbound DNS configuration checker utility.
endef

define Package/unbound-control
  $(call Package/unbound/Default)
  TITLE+= (remote control)
  DEPENDS+= +unbound-daemon
endef

define Package/unbound-control/description
  This package contains the Unbound control utility.
endef

define Package/unbound-control-setup
  $(call Package/unbound/Default)
  TITLE+= (control setup)
  DEPENDS+= +unbound-control +openssl-util
endef

define Package/unbound-control-setup/description
  This package contains the Unbound control setup utility.
endef

define Package/unbound-host
  $(call Package/unbound/Default)
  TITLE+= (DNS lookup)
  DEPENDS+= +libunbound
endef

define Package/unbound-host/description
  This package contains the Unbound DNS lookup utility.
endef

define Package/libunbound/config
	if PACKAGE_libunbound
	config PACKAGE_libunbound_dnscrypt
		bool "Build with DNSCRYPT support."
		default n
	config PACKAGE_libunbound_ipset
		bool "Build with IPSET (libmnl) support."
		default y
	config PACKAGE_libunbound_libevent
		bool "Build with expanded network resource (libevent) support."
		default y
	config PACKAGE_libunbound_libpthread
		bool "Build with POSIX threading (libpthread) support."
		default y
	config PACKAGE_libunbound_nghttp2
		bool "Build with DNS over HTTPS support (nghttp2 framework)."
		default n
	config PACKAGE_libunbound_pythonmodule
		bool "Build with PYTHON module for prototyping and data analysis."
		default n
	config PACKAGE_libunbound_subnet
		bool "Build with SUBNET cache module support."
		default n
	config PACKAGE_libunbound_dnstap
		bool "Build with dnstap support."
		default n
	endif
endef

CONFIGURE_ARGS += \
	--disable-dsa \
	--disable-gost \
	--enable-allsymbols \
	--enable-ecdsa \
	--enable-tfo-client \
	--enable-tfo-server \
	--with-libexpat="$(STAGING_DIR)/usr" \
	--with-ssl="$(STAGING_DIR)/usr" \
	--with-user=unbound \
	--with-run-dir=/var/lib/unbound \
	--with-conf-file=/var/lib/unbound/unbound.conf \
	--with-pidfile=/var/run/unbound.pid \
    $(if $(CONFIG_PACKAGE_libunbound_dnscrypt), \
        --enable-dnscrypt --with-libsodium="$(STAGING_DIR)/usr",) \
    $(if $(CONFIG_PACKAGE_libunbound_ipset), \
        --enable-ipset --with-libmnl="$(STAGING_DIR)/usr",) \
    $(if $(CONFIG_PACKAGE_libunbound_libevent), \
        --enable-event-api --with-libevent="$(STAGING_DIR)/usr",--without-libevent) \
    $(if $(CONFIG_PACKAGE_libunbound_nghttp2), \
        --with-libnghttp2="$(STAGING_DIR)/usr",) \
    $(if $(CONFIG_PACKAGE_libunbound_libpthread), \
        --with-pthreads,--without-pthreads --without-solaris-threads) \
    $(if $(CONFIG_PACKAGE_libunbound_python),--with-pythonmodule,) \
    $(if $(CONFIG_PACKAGE_libunbound_subnet),--enable-subnet,) \
    $(if $(CONFIG_PACKAGE_libunbound_dnstap),--enable-dnstap,) \

define Package/unbound-daemon/conffiles
/etc/config/unbound
/etc/unbound/unbound.conf
/etc/unbound/unbound_ext.conf
/etc/unbound/unbound_srv.conf
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libunbound.{so*,a,la} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/unbound.h $(1)/usr/include/
ifneq ($(CONFIG_PACKAGE_libunbound_libevent),)
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/unbound-event.h $(1)/usr/include/
endif
endef

define Package/unbound-daemon/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/sbin/unbound $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/unbound
	$(INSTALL_CONF) \
		$(PKG_INSTALL_DIR)/var/lib/unbound/unbound.conf \
		$(1)/etc/unbound/unbound.conf
	$(INSTALL_DATA) ./files/root.key $(1)/etc/unbound/root.key
	$(INSTALL_CONF) ./files/unbound_ext.conf $(1)/etc/unbound/unbound_ext.conf
	$(INSTALL_CONF) ./files/unbound_srv.conf $(1)/etc/unbound/unbound_srv.conf
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/unbound.uci $(1)/etc/config/unbound
	$(INSTALL_DIR) $(1)/etc/hotplug.d/ntp
	$(INSTALL_BIN)  ./files/unbound.ntpd $(1)/etc/hotplug.d/ntp/25-unbound
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN)  ./files/unbound.init $(1)/etc/init.d/unbound
	$(INSTALL_DIR) $(1)/usr/lib/unbound
	$(INSTALL_DATA) ./files/defaults.sh $(1)/usr/lib/unbound/defaults.sh
	$(INSTALL_DATA) ./files/dnsmasq.sh $(1)/usr/lib/unbound/dnsmasq.sh
	$(INSTALL_DATA) ./files/iptools.sh $(1)/usr/lib/unbound/iptools.sh
	$(INSTALL_BIN)  ./files/odhcpd.sh $(1)/usr/lib/unbound/odhcpd.sh
	$(INSTALL_DATA) ./files/odhcpd.awk $(1)/usr/lib/unbound/odhcpd.awk
	$(INSTALL_DATA) ./files/stopping.sh $(1)/usr/lib/unbound/stopping.sh
	$(INSTALL_DATA) ./files/unbound.sh $(1)/usr/lib/unbound/unbound.sh
endef

define Package/libunbound/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libunbound.so.* $(1)/usr/lib/
endef

define Package/unbound-anchor/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/unbound-anchor $(1)/usr/sbin/
endef

define Package/unbound-checkconf/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/unbound-checkconf $(1)/usr/sbin/
endef

define Package/unbound-control/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/unbound-control $(1)/usr/sbin/
endef

define Package/unbound-control-setup/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/unbound-control-setup $(1)/usr/sbin/
endef

define Package/unbound-host/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/unbound-host $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,unbound-daemon))
$(eval $(call BuildPackage,libunbound))
$(eval $(call BuildPackage,unbound-anchor))
$(eval $(call BuildPackage,unbound-checkconf))
$(eval $(call BuildPackage,unbound-control))
$(eval $(call BuildPackage,unbound-control-setup))
$(eval $(call BuildPackage,unbound-host))

