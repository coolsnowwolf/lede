include $(TOPDIR)/rules.mk

PKG_NAME:=pdns
PKG_VERSION:=4.4.1
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://downloads.powerdns.com/releases/
PKG_HASH:=03fa7c181c666a5fc44a49affe7666bd385d46c1fe15088caff175967e85ab6c

PKG_MAINTAINER:=Peter van Dijk <peter.van.dijk@powerdns.com>
PKG_LICENCE:=GPL-2.0-only
PKG_LICENCE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:powerdns:authoritative

PKG_FIXUP:=autoreconf

PKG_INSTALL:=1

PKG_BUILD_PARALLEL:=1

PKG_BUILD_DEPENDS:=unixodbc/host protobuf-c/host

include $(INCLUDE_DIR)/package.mk

define Package/pdns/Default
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=IP Addresses and Names
  TITLE:=PowerDNS Authoritative Server
  DEPENDS:=+libatomic
  URL:=https://www.powerdns.com/auth.html
endef

define Package/pdns/description
  PowerDNS is a versatile nameserver which supports a large number
  of different backends ranging from simple zonefiles to relational
  databases and load balancing/failover algorithms.
  PowerDNS tries to emphasize speed and security.

  This is the authoritative nameserver that answers questions about
  domains that it knows about. You also need at least one backend installed to
  serve data.
endef

PDNS_BACKENDS =

define Package/pdns-backends
$(call Package/pdns/Default)
  TITLE+= (all backends)
  DEPENDS+= $(PDNS_DEPENDS)
  HIDDEN:=1
endef

define Package/pdns-backends/description
 This meta package contains only dependencies for PowerDNS backends.
endef

# Create a meta-package of dependent backends (for ALL)
define Package/pdns-backends/install
  true
endef

define Package/pdns-tools
  $(call Package/pdns/Default)
  TITLE:=Tools for DNS debugging by PowerDNS
  DEPENDS+=+boost +boost-program_options +libcurl +libmbedtls +libopenssl +p11-kit +protobuf
endef

define Package/pdns-tools/description
  PowerDNS is a versatile nameserver which supports a large number
  of different backends ranging from simple zonefiles to relational
  databases and load balancing/failover algorithms.
  PowerDNS tries to emphasize speed and security.

  This package contains several tools to debug DNS issues. These tools do not
  require any part of the PowerDNS server components to work.

    * dnsbulktest: A resolver stress-tester
    * dnsgram: Show per 5-second statistics to study intermittent resolver issues
    * dnsreplay: Replay a pcap with DNS queries
    * dnsscan: Prints the query-type amounts in a pcap
    * dnsscope: Calculates statistics without replaying traffic
    * dnstcpbench: Perform TCP benchmarking of DNS servers
    * dnswasher: Clean a pcap of identifying IP information
    * ixplore: Explore diffs from IXFRs
    * nsec3dig: Calculate the correctness of NSEC3 proofs
    * saxfr: AXFR zones and show extra information
endef

define Package/pdns-ixfrdist
  $(call Package/pdns/Default)
  TITLE:=Redistribute zones via AXFR and IXFR
  DEPENDS+=+boost +boost-program_options +libopenssl +libyaml-cpp +p11-kit +protobuf
endef

define Package/pdns-ixfrdist/description
  PowerDNS is a versatile nameserver which supports a large number
  of different backends ranging from simple zonefiles to relational
  databases and load balancing/failover algorithms.
  PowerDNS tries to emphasize speed and security.

  This package contains the ixfrdist tool that can redistribute multiple versions
  of zones over AXFR and IXFR. This tool does not require any part of the
  PowerDNS server components to work.
endef

define Package/pdns
  $(call Package/pdns/Default)
  DEPENDS+=+boost +boost-program_options +libcurl +liblua +libopenssl +libsqlite3 +p11-kit
endef

define Package/pdns/config
	menu "Select PowerDNS backends"
		depends on PACKAGE_pdns
		comment "PowerDNS backends"

		$(foreach backend,$(PDNS_BACKENDS), \
			config PACKAGE_pdns-backend-$(backend)
			prompt "$(PDNS_BACKEND_$(backend)_DESC) backend"
			default m if ALL
		)
	endmenu
endef


# 1: short name
# 2: dependencies on other PowerDNS libraries (short name)
# 3: dependencies on other packages
# 4: conditional/inward dependencies
# 5: friendly display name
define DefinePdnsBackend
  PDNS_DEPENDS+= +pdns-backend-$(1)
  PKG_CONFIG_DEPENDS+= CONFIG_PACKAGE_pdns-backend$(1)

  PDNS_BACKENDS+= $(1)

  PDNS_BACKEND_$(1)_DESC=$(if $(5),$(5),$(1))
  PDNS_BACKEND_$(1)_LIB=$(if $(4),$(4),$(1))

  define Package/pdns-backend-$(1)
    $(call Package/pdns/Default)
    TITLE+= ($(1))
    DEPENDS+= pdns $$(foreach backend,$(2),+pdns-backend-$$(backend)) \
	      $$(foreach dependency,$(3),+$$(dependency))
    HIDDEN:=1
  endef

  define Package/pdns-backend-$(1)/description
   PowerDNS is a versatile nameserver which supports a large number
   of different backends ranging from simple zonefiles to relational
   databases and load balancing/failover algorithms.
   PowerDNS tries to emphasize speed and security.

   This package contains the $(if $(5),$(5),$(1)) backend for the PowerDNS nameserver.
  endef
endef

$(eval $(call DefinePdnsBackend,geoip,,libmaxminddb libyaml-cpp,,GeoIP))
$(eval $(call DefinePdnsBackend,mysql,,libmysqlclient,gmysql,MySQL))
$(eval $(call DefinePdnsBackend,odbc,,unixodbc,godbc,ODBC))
$(eval $(call DefinePdnsBackend,pgsql,,libpq,gpgsql,PostgreSQL))
$(eval $(call DefinePdnsBackend,sqlite3,,libsqlite3,gsqlite3,SQLite 3))
$(eval $(call DefinePdnsBackend,ldap,,libopenldap krb5-libs,,OpenLDAP))
$(eval $(call DefinePdnsBackend,lua2,,lua liblua,,Lua2))
$(eval $(call DefinePdnsBackend,pipe,,,,Pipe))
$(eval $(call DefinePdnsBackend,remote,,,,Remote))

define Package/pdns/conffiles
/etc/powerdns/pdns.conf
/etc/init.d/pdns
endef

CONFIGURE_ARGS+= \
	--sysconfdir=/etc/powerdns \
	--libdir=/usr/lib/powerdns  \
	--with-dynmodules="$(foreach backend,$(PDNS_BACKENDS),$(if $(CONFIG_PACKAGE_pdns-backend-$(backend)),$(PDNS_BACKEND_$(backend)_LIB),))" \
	--with-lua="lua" \
	--with-modules="bind random" \
	--with-mysql-lib=$(STAGING_DIR)/usr \
	--with-mysql-includes=$(STAGING_DIR)/usr \
	$(if $(CONFIG_PACKAGE_pdns-tools),--enable-tools,) \
	--with-protobuf \
	--without-libsodium \
	--enable-experimental-pkcs11 \
	--enable-lua-records \
	--enable-reproducible \
	$(if $(CONFIG_PACKAGE_pdns-ixfrdist),--enable-ixfrdist,)

define Package/pdns/install
	$(INSTALL_DIR) $(1)/etc/powerdns
	$(INSTALL_CONF) ./files/pdns.conf-dist $(1)/etc/powerdns/pdns.conf-dist
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/pdns.init $(1)/etc/init.d/pdns
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/pdns_control $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/pdnsutil $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/zone2sql $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/zone2json $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/pdns_server $(1)/usr/sbin/
endef

define Package/pdns/Default/install
	$(INSTALL_DIR) $(1)/usr/lib/powerdns/pdns
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/powerdns/pdns/lib$(PDNS_BACKEND_$(2)_LIB)backend.so $(1)/usr/lib/powerdns/pdns/
	if [ "$(PDNS_BACKEND_$(2)_LIB)" == "ldap" ]; then \
	$(INSTALL_DIR) $(1)/usr/bin; \
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/zone2ldap $(1)/usr/bin/; \
	fi
endef

define Package/pdns-ixfrdist/install
	$(INSTALL_DIR) $(1)/etc/powerdns
	$(CP) $(PKG_INSTALL_DIR)/etc/powerdns/ixfrdist.example.yml $(1)/etc/powerdns/
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/ixfrdist $(1)/usr/bin/
endef

define Package/pdns-tools/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/dnsgram $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/dnsreplay $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/dnsscan $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/dnsscope $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/dnswasher $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/dumresp $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/pdns_notify $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/nproxy $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/nsec3dig $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/saxfr $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/stubquery $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/ixplore $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/sdig $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/calidns $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/dnsbulktest $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/dnstcpbench $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/dnspcap2protobuf $(1)/usr/bin/
endef

define BuildPdnsBackend
  define Package/pdns-backend-$(1)/install
	$(call Package/pdns/Default/install,$$(1),$(1))
  endef

  $$(eval $$(call BuildPackage,pdns-backend-$(1)))
endef

$(foreach backend,$(PDNS_BACKENDS),$(eval $(call BuildPdnsBackend,$(backend))))
$(eval $(call BuildPackage,pdns))
$(eval $(call BuildPackage,pdns-tools))
$(eval $(call BuildPackage,pdns-ixfrdist))
