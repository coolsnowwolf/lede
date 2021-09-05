include $(TOPDIR)/rules.mk

PKG_NAME:=dnsdist
PKG_VERSION:=1.6.0
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://downloads.powerdns.com/releases/
PKG_HASH:=a7783a04d8d4ad2b0168ffaaf85ef95d5f557057b0462280684dd799d0cdd292

PKG_MAINTAINER:=Peter van Dijk <peter.van.dijk@powerdns.com>
PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:powerdns:dnsdist

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

PKG_ASLR_PIE:=0
PKG_BUILD_DEPENDS:=boost

PKG_CONFIG_DEPENDS:= \
  CONFIG_DNSDIST_GNUTLS \
  CONFIG_DNSDIST_OPENSSL

include $(INCLUDE_DIR)/package.mk

define Package/dnsdist/config
menu "Configuration"
	depends on PACKAGE_dnsdist

	comment "SSL Support"
	choice
		prompt "Selected SSL library"
		default DNSDIST_OPENSSL

		config DNSDIST_OPENSSL
			bool "OpenSSL"

		config DNSDIST_GNUTLS
			bool "GnuTLS"

		config DNSDIST_NOSSL
			bool "No SSL support"

	endchoice

	comment "DNS over HTTPS/TLS Support"
	depends on !DNSDIST_NOSSL

	config DNSDIST_DNS_OVER_HTTPS
	depends on DNSDIST_OPENSSL
	depends on !DNSDIST_NOSSL
		bool "DNS over HTTPS Support"
		help
			"Enables DNS over HTTPS Support for dnsdist"
		default y

	config DNSDIST_DNS_OVER_TLS
	depends on !DNSDIST_NOSSL
		bool "DNS over TLS Support"
		help
			"Enabled DNS over TLS Support for dnsdist"
		default y

	config DNSDIST_NET_SNMP
		bool "Net-SNMP support"
		help
			"Enable Net-SNMP support for dnsdist"
		default y

	config DNSDIST_RE2
		bool "RE2 support"
		help
			"Enable RE2 support for dnsdist"
		default y

	config DNSDIST_DNSTAP
		bool "DNSTAP support"
		help
			"Enable DNSTAP support for dnsdist"
		default y

	config DNSDIST_SODIUM
		bool "Build with libsodium"
		help
			"Build with libsodium - for encrypted console connections, and DNSCrypt"
		default y
endmenu
endef

define Package/dnsdist
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=IP Addresses and Names
  TITLE:=dnsdist DNS-, DOS- and abuse-aware loadbalancer
  DEPENDS:= \
	  +DNSDIST_DNS_OVER_HTTPS:libh2o-evloop \
	  +DNSDIST_GNUTLS:libgnutls \
	  +DNSDIST_OPENSSL:libopenssl \
	  +DNSDIST_NET_SNMP:libnetsnmp \
	  +DNSDIST_RE2:re2 \
	  +DNSDIST_DNSTAP:libfstrm \
	  +DNSDIST_SODIUM:libsodium \
	  +libatomic \
	  +libcap \
	  +libedit \
	  +libstdcpp \
	  +lmdb \
	  +liblua \
	  +tinycdb
  URL:=https://dnsdist.org/
endef

define Package/dnsdist/description
  dnsdist is a highly DNS-, DoS- and abuse-aware loadbalancer. Its goal in life
  is to route traffic to the best server, delivering top performance to legitimate
  users while shunting or blocking abusive traffic.
endef

define Package/dnsdist/conffiles
/etc/dnsdist.conf
/etc/config/dnsdist
/etc/init.d/dnsdist
endef

# not everything groks --disable-nls
DISABLE_NLS:=

# OpenWRT's setting of CXX destroys dnsdist's -std=c++17
# --with-re2 compensates for that because it compensates for a bug in re2.pc that also destroys it
# so this addition is for the --without-re2 case
#
# none of this is pretty
TARGET_CXX+=-std=c++17

CONFIGURE_ARGS+= \
	--enable-option-checking=fatal \
	--with-pic \
	--with-lua=lua \
	$(if $(CONFIG_DNSDIST_SODIUM),--enable-dnscrypt --with-libsodium,--disable-dnscrypt --without-libsodium) \
	$(if $(CONFIG_DNSDIST_DNSTAP),--enable-dnstap=yes,--enable-dnstap=no) \
	$(if $(CONFIG_DNSDIST_RE2),--with,--without)-re2 \
	$(if $(CONFIG_DNSDIST_NET_SNMP),--with,--without)-net-snmp \
	$(if $(CONFIG_DNSDIST_GNUTLS),--with,--without)-gnutls \
	$(if $(CONFIG_DNSDIST_OPENSSL),--with,--without)-libssl \
	$(if $(CONFIG_DNSDIST_DNS_OVER_TLS),--enable-dns-over-tls,) \
	$(if $(CONFIG_DNSDIST_DNS_OVER_HTTPS),--enable-dns-over-https,)

define Package/dnsdist/install
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_CONF) ./files/dnsdist.conf $(1)/etc/dnsdist.conf
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/dnsdist.config $(1)/etc/config/dnsdist
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/dnsdist.init $(1)/etc/init.d/dnsdist
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dnsdist $(1)/usr/bin/
endef

$(eval $(call BuildPackage,dnsdist))
