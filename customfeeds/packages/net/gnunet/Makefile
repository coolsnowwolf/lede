include $(TOPDIR)/rules.mk

PKG_NAME:=gnunet

PKG_VERSION:=0.14.1
PKG_RELEASE:=$(AUTORELEASE)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/gnunet
PKG_HASH:=4a3205c570c30756f1a8b1ad0f1a63d078a92f0fac8e543471d54f4552da18c2

PKG_LICENSE:=AGPL-3.0
PKG_LICENSE_FILES:=COPYING
PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>
PKG_CPE_ID:=cpe:/a:gnunet:gnunet

PKG_BUILD_PARALLEL:=1
PKG_FIXUP:=gettext-version
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

CONFIGURE_ARGS+= \
	--with-libiconv-prefix="$(ICONV_PREFIX)" \
	--with-libintl-prefix="$(INTL_PREFIX)" \
	--with-ltdl \
	$(if $(CONFIG_PACKAGE_$(PKG_NAME)-mysql),--with-mysql="$(STAGING_DIR)/usr",--without-mysql) \
	$(if $(CONFIG_PACKAGE_$(PKG_NAME)-pgsql),--with-postgresql="$(STAGING_DIR)/usr/bin/pg_config",--without-postgresql) \
	$(if $(CONFIG_PACKAGE_libsqlite3),--with-sqlite="$(STAGING_DIR)/usr",--without-sqlite) \
	--enable-testing \
	--disable-testruns \
	--disable-documentation \
	--enable-experimental \
	--with-extractor=$(STAGING_DIR)/usr \
	--with-gnutls=$(STAGING_DIR)/usr \
	$(if $(CONFIG_PACKAGE_$(PKG_NAME)-transport-bluetooth),--with-libbluetooth="$(STAGING_DIR)/usr",--without-libbluetooth) \
	--with-libgnurl=$(STAGING_DIR)/usr \
	--with-libogg=$(STAGING_DIR)/usr \
	--with-libopus=$(STAGING_DIR)/usr \
	--with-libpulse=$(STAGING_DIR)/usr \
	--with-libunistring-prefix=$(STAGING_DIR)/usr \
	--with-microhttpd=$(STAGING_DIR)/usr

# upstream now provides --with-pulseaudio but doesn't detect rpath
TARGET_LDFLAGS+= -Wl,-rpath-link=$(STAGING_DIR)/usr/lib/pulseaudio

define Package/gnunet/Default
  SECTION:=net
  CATEGORY:=Network
  TITLE:=GNUnet
  URL:=https://www.gnunet.org/
endef

define Package/gnunet
$(call Package/gnunet/Default)
  TITLE+= - a peer-to-peer framework focusing on security
  DEPENDS:=+libatomic +libgcrypt +libgpg-error +libidn2 +libltdl +libsodium \
           +libunistring +librt +zlib $(ICONV_DEPENDS) $(INTL_DEPENDS)
  USERID:=gnunet=958:gnunet=958
  MENU:=1
  FILE_MODES:=/usr/lib/gnunet/libexec/gnunet-helper-nat-server:root:gnunet:4750 \
              /usr/lib/gnunet/libexec/gnunet-helper-nat-client:root:gnunet:4750
endef

define Package/gnunet/description
 GNUnet is a peer-to-peer framework focusing on security. The first and
 primary application for GNUnet is anonymous file-sharing.  GNUnet is
 currently developed by a worldwide group of independent free software
 developers.  GNUnet is a GNU package (http://www.gnu.org/).

 This is an ALPHA release.  There are known and significant bugs as
 well as many missing features in this release.

 This package provides the core components of GNUnet including the
 CADET routing engine, a DHT implementation and basic transports as
 well as their helpers.
endef

define BuildComponent
  PKG_CONFIG_DEPENDS+=CONFIG_PACKAGE_$(PKG_NAME)-$(1)

  define Package/gnunet-$(1)
    $$(call Package/gnunet/Default)
    TITLE+= $(2)
    DEPENDS:=gnunet $(DEPENDS_$(1))
    $(if $(3),DEFAULT:=y if PACKAGE_gnunet)
    $(if $(USERID_$(1)),USERID:=$(USERID_$(1)))
    $(if $(CONFLICTS_$(1)),CONFLICTS:=$(CONFLICTS_$(1)))
    $(if $(FILE_MODES_$(1)),FILE_MODES:=$(FILE_MODES_$(1)))
  endef

  define Package/gnunet-$(1)/install
	( if [ "$(BIN_$(1))" ]; then \
		$(INSTALL_DIR) $$(1)/usr/bin ; \
		for bin in $(BIN_$(1)); do \
			$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/gnunet-$$$$$$$$bin $$(1)/usr/bin/ ; \
		done \
	fi )

	( if [ "$(LIB_$(1))" ]; then \
		$(INSTALL_DIR) $$(1)/usr/lib ; \
		for lib in $(LIB_$(1)); do \
			$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgnunet$$$$$$$$lib.so* $$(1)/usr/lib/ ; \
		done \
	fi )

	( if [ "$(PLUGIN_$(1))" ]; then \
		$(INSTALL_DIR) $$(1)/usr/lib/gnunet ; \
		for plug in $(PLUGIN_$(1)); do \
			$(CP) $(PKG_INSTALL_DIR)/usr/lib/gnunet/libgnunet_plugin_$$$$$$$$plug*.so $$(1)/usr/lib/gnunet ; \
		done \
	fi )

	( if [ "$(LIBEXEC_$(1))" ]; then \
		$(INSTALL_DIR) $$(1)/usr/lib/gnunet/libexec ; \
		for lex in $(LIBEXEC_$(1)); do \
			$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/gnunet/libexec/gnunet-$$$$$$$$lex $$(1)/usr/lib/gnunet/libexec ; \
		done \
	fi )

	( if [ "$(CONF_$(1))" ]; then \
		$(INSTALL_DIR) $$(1)/usr/share/gnunet/config.d ; \
		for conf in $(CONF_$(1)); do \
		$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/gnunet/config.d/$$$$$$$$conf.conf $$(1)/usr/share/gnunet/config.d ; \
		done \
	fi )

	( if [ -e ./files/gnunet-$(1).defaults ]; then \
		$(INSTALL_DIR) $$(1)/etc/uci-defaults ; \
		$(INSTALL_BIN) ./files/gnunet-$(1).defaults $$(1)/etc/uci-defaults/gnunet-$(1) ; \
	fi )
  endef

  $$(eval $$(call BuildPackage,gnunet-$(1)))
endef

define Package/gnunet/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/lib/gnunet/libexec
	$(INSTALL_DIR) $(1)/usr/share/gnunet/config.d $(1)/usr/share/gnunet/hellos

	( for bin in arm ats cadet core config ecc identity nat nat-auto nat-server nse \
	    peerinfo peerstore revocation scalarproduct scrypt statistics transport uri; do \
		$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/gnunet-$$$$bin $(1)/usr/bin/ ; \
	done )

	( for lib in arm ats block blockgroup cadet \
	    core datacache dht dns fragmentation friends hello \
	    identity natauto natnew nse nt peerinfo peerstore regexblock regex revocation \
	    scalarproduct set seti setu statistics transport transportapplication \
	    transportcore transportmonitor util; do \
		$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgnunet$$$$lib.so* $(1)/usr/lib/ ; \
	done )

	( for plug in ats_proportional block_dht block_regex block_revocation transport_unix; do \
		$(CP) $(PKG_INSTALL_DIR)/usr/lib/gnunet/libgnunet_plugin_$$$$plug*.so $(1)/usr/lib/gnunet ; \
	done )

	( for lex in daemon-topology helper-nat-client \
	    helper-nat-server service-arm service-ats service-ats-new service-cadet \
	    service-core service-dht service-identity service-nat service-nat-auto \
	    service-nse service-peerinfo service-peerstore service-regex \
	    service-revocation service-scalarproduct-alice \
	    service-scalarproduct-bob service-scalarproduct-ecc-alice \
	    service-scalarproduct-ecc-bob service-set service-seti service-setu service-statistics \
	    service-transport timeout; do \
		$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/gnunet/libexec/gnunet-$$$$lex $(1)/usr/lib/gnunet/libexec ; \
	done )

	( for conf in arm ats cadet core datacache dht identity \
	    nat nat-auto nse peerinfo peerstore regex revocation \
	    scalarproduct set seti setu statistics topology transport util; do \
		$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/gnunet/config.d/$$$$conf.conf $(1)/usr/share/gnunet/config.d ; \
	done )

	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/gnunet/hellos/* $(1)/usr/share/gnunet/hellos
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/gnunet.init $(1)/etc/init.d/gnunet
	$(INSTALL_DIR) $(1)/lib/upgrade/keep.d
	$(INSTALL_DATA) ./files/gnunet.upgrade $(1)/lib/upgrade/keep.d/gnunet
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/gnunet.defaults $(1)/etc/uci-defaults/gnunet
	$(INSTALL_DIR) $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./files/gnunet-proto.sh $(1)/lib/netifd/proto/gnunet.sh
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/gnunet $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.{la,so}* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/include/gnunet/*.h $(1)/usr/include/gnunet
endef

BIN_abd:=abd
LIB_abd:=abd
PLUGIN_abd:=gnsrecord_abd
LIBEXEC_abd:=service-abd
CONF_abd:=abd
DEPENDS_abd:=+gnunet-gns

BIN_auction:=auction-create auction-info auction-join
LIBEXEC_auction:=service-auction
CONF_auction:=auction
DEPENDS_auction:=+gnunet-reclaim +jansson

DEPENDS_conversation:=+gnunet-gns +libgst1app +libgst1audio +libgstreamer1 +glib2 +pulseaudio-daemon +libopus +libogg
BIN_conversation:=conversation conversation-test
LIB_conversation:=conversation microphone speaker
PLUGIN_conversation:=gnsrecord_conversation
LIBEXEC_conversation:=helper-audio-playback helper-audio-record service-conversation
CONF_conversation:=conversation

BIN_dht-cli:=dht-get dht-put dht-monitor
PLUGIN_dht-cli:=block_test

DEPENDS_curl:=+libgnurl +jansson
LIB_curl:=curl

DEPENDS_hostlist:=+libmicrohttpd-ssl +gnunet-curl +ca-certificates
LIBEXEC_hostlist:=daemon-hostlist
CONF_hostlist:=hostlist

DEPENDS_transport-bluetooth:=+bluez-libs
PLUGIN_transport-bluetooth:=transport_bluetooth
LIBEXEC_transport-bluetooth:=helper-transport-bluetooth
FILE_MODES_transport-bluetooth:=/usr/lib/gnunet/libexec/gnunet-helper-transport-bluetooth:root:gnunet:4750

DEPENDS_transport-http_client:=+gnunet-curl +ca-certificates
PLUGIN_transport-http_client:=transport_http_client transport_https_client

DEPENDS_transport-http_server:=+libmicrohttpd-ssl
PLUGIN_transport-http_server:=transport_http_server transport_https_server

PLUGIN_transport-tcp:=transport_tcp

PLUGIN_transport-udp:=transport_udp

PLUGIN_transport-wlan:=transport_wlan
LIBEXEC_transport-wlan:=helper-transport-wlan
FILE_MODES_transport-wlan:=/usr/lib/gnunet/libexec/gnunet-helper-transport-wlan:root:gnunet:4750

# BIN_dv:=dv
LIB_dv:=dv
PLUGIN_dv:=transport_dv
LIBEXEC_dv:=service-dv
CONF_dv:=dv

DEPENDS_fs:=+gnunet-datastore +libextractor
BIN_fs:=auto-share directory download fs publish unindex search
LIB_fs:=fs
PLUGIN_fs:=block_fs
LIBEXEC_fs:=helper-fs-publish service-fs
CONF_fs:=fs

DEPENDS_gns:=+gnunet-vpn +iptables-mod-extra
USERID_gns:=:gnunetdns=452
BIN_gns:=gns gns-import.sh namecache namestore resolver zoneimport 
LIB_gns:=gns gnsrecord namecache namestore
PLUGIN_gns:=block_dns block_gns gnsrecord_conversation gnsrecord_dns gnsrecord_gns
LIBEXEC_gns:=dns2gns helper-dns service-dns service-gns service-namecache service-namestore service-resolver service-zonemaster
CONF_gns:=dns gns namecache namestore resolver zonemaster
FILE_MODES_gns:=/usr/lib/gnunet/libexec/gnunet-helper-dns:root:gnunetdns:4750 /usr/lib/gnunet/libexec/gnunet-service-dns:gnunet:gnunetdns:2750

DEPENDS_namestore-fcfsd:=+gnunet-gns +libmicrohttpd-ssl
LIBEXEC_namestore-fcfsd:=namestore-fcfsd

DEPENDS_gns-proxy:=+gnunet-gns +gnunet-curl +libmicrohttpd-ssl +PACKAGE_libgnutls-dane:libgnutls-dane
LIBEXEC_gns-proxy:=gns-proxy

DEPENDS_datastore:=+gnunet-gns
BIN_datastore:=datastore
LIB_datastore:=datastore
LIBEXEC_datastore:=service-datastore
CONF_datastore:=datastore

BIN_messenger:=messenger
LIB_messenger:=messenger
LIBEXEC_messenger:=service-messenger
CONF_messenger:=messenger

DEPENDS_reclaim:=+gnunet-gns +gnunet-sqlite +libpbc +libgabe +jansson
BIN_reclaim:=reclaim
LIB_reclaim:=consensus abe reclaim reclaimattribute secretsharing
LIBEXEC_reclaim:=service-consensus service-reclaim service-secretsharing
CONF_reclaim:=consensus reclaim secretsharing
PLUGIN_reclaim:=block_consensus gnsrecord_reclaim reclaim_credential_jwt reclaim_attribute_basic

DEPENDS_rest:=+gnunet-gns +gnunet-reclaim +libmicrohttpd-ssl +jansson
LIB_rest:=rest json gnsrecordjson
PLUGIN_rest:=rest_config rest_copying rest_gns rest_identity rest_namestore rest_peerinfo rest_openid_connect rest_reclaim
LIBEXEC_rest:=rest-server
CONF_rest:=rest

BIN_rps:=rps
LIB_rps:=rps
LIBEXEC_rps:=service-rps
CONF_rps:=rps

PLUGIN_dhtcache-heap:=datacache_heap
CONFLICTS_dhtcache-heap:=gnunet-dhtcache-pgsql gnunet-dhtcache-sqlite

DEPENDS_gns-flat:=+gnunet-gns
PLUGIN_gns-flat:=namecache_flat namestore_flat

PLUGIN_peerstore-flat:=peerstore_flat

DEPENDS_fs-heap:=+gnunet-datastore
PLUGIN_fs-heap:=datastore_heap
CONFLICTS_fs-heap:=gnunet-fs-mysql gnunet-fs-pgsql gnunet-fs-sqlite

DEPENDS_mysql:=+libmysqlclient
LIB_mysql:=mysql my


DEPENDS_fs-mysql:=+gnunet-mysql +gnunet-datastore
PLUGIN_fs-mysql:=datastore_mysql
CONFLICTS_fs-mysql:=gnunet-fs-pgsql gnunet-fs-sqlite

DEPENDS_pgsql:=+libpq
LIB_pgsql:=pq

DEPENDS_dhtcache-pgsql:=+gnunet-pgsql
PLUGIN_dhtcache-pgsql:=datacache_postgres
CONFLICTS_dhtcache-pgsql:=gnunet-dhtcache-sqlite

DEPENDS_fs-pgsql:=+gnunet-pgsql +gnunet-datastore
PLUGIN_fs-pgsql:=datastore_postgres
CONFLICTS_fs-pgsql:=gnunet-fs-sqlite

DEPENDS_gns-pgsql:=+gnunet-pgsql +gnunet-gns
PLUGIN_gns-pgsql:=namecache_postgres namestore_postgres
CONFLICTS_gns-pgsql:=gnunet-gns-sqlite gnunet-gns-flat

DEPENDS_sqlite:=+libsqlite3
LIB_sqlite:=sq

DEPENDS_gns-sqlite:=+gnunet-gns +gnunet-sqlite
PLUGIN_gns-sqlite:=namecache_sqlite namestore_sqlite
CONFLICTS_gns-sqlite:=gnunet-gns-flat

DEPENDS_peerstore-sqlite:=+gnunet-sqlite
PLUGIN_peerstore-sqlite:=peerstore_sqlite
CONFLICTS_peerstore-sqlite:=gnunet-peerstore-flat

DEPENDS_dhtcache-sqlite:=+gnunet-sqlite
PLUGIN_dhtcache-sqlite:=datacache_sqlite

DEPENDS_fs-sqlite:=+gnunet-datastore +gnunet-sqlite
PLUGIN_fs-sqlite:=datastore_sqlite

DEPENDS_utils:=+certtool +openssl-util
BIN_utils:=gns-proxy-setup-ca transport-certificate-creation

DEPENDS_vpn:=+kmod-tun +iptables +firewall
BIN_vpn:=vpn
FILE_MODES_vpn:=/usr/lib/gnunet/libexec/gnunet-helper-exit:root:gnunet:4750 /usr/lib/gnunet/libexec/gnunet-helper-vpn:root:gnunet:4750
LIB_vpn:=dnsstub tun vpn
LIBEXEC_vpn:=daemon-exit daemon-pt helper-exit helper-vpn service-vpn
CONF_vpn:=exit pt vpn

define Package/gnunet-gns/prerm
#!/bin/sh

uci -q batch <<EOF
del network.gnunetdns
del network.gndnsrtt
del network.gndnsrl
commit network

del firewall.gnunetdns
del firewall.gndnsrl
del firewall.gndnsrl2
commit firewall
EOF
endef

define Package/gnunet-vpn/prerm
#!/bin/sh

uci -q batch <<EOF
del network.gnunetvpn
del network.gnunetexit
commit network
del firewall.gnunetvpn
del firewall.gnunetexit
del firewall.gnexitfwd
commit firewall
EOF
endef

define PostInstFixSUIDPerms
  define Package/$(1)/postinst
#!/bin/sh
[ -z "$IPKG_INSTROOT" ] || exit 0
[ -e /usr/share/gnunet/.permfix ] && rm /usr/share/gnunet/.permfix || exit 0
  endef
endef

$(eval $(call PostInstFixSUIDPerms,gnunet))
$(eval $(call PostInstFixSUIDPerms,gnunet-gns))
$(eval $(call PostInstFixSUIDPerms,gnunet-transport-bluetooth))
$(eval $(call PostInstFixSUIDPerms,gnunet-transport-wlan))
$(eval $(call PostInstFixSUIDPerms,gnunet-vpn))

$(eval $(call BuildPackage,gnunet))
#$(eval $(call BuildComponent,abd,credential service components,))
$(eval $(call BuildComponent,auction,auction components,))
$(eval $(call BuildComponent,conversation,conversation component,))
$(eval $(call BuildComponent,curl,cURL wrapper component,))
$(eval $(call BuildComponent,datastore,data storage components,))
$(eval $(call BuildComponent,dht-cli,DHT command line clients,))
# $(eval $(call BuildComponent,dv,distance-vector routing component,y))
$(eval $(call BuildComponent,fs,file-sharing components,))
$(eval $(call BuildComponent,gns,name resolution components,y))
$(eval $(call BuildComponent,gns-proxy,gns-proxy component,))
$(eval $(call BuildComponent,hostlist,HTTP bootstrap hostlist client and server,y))
$(eval $(call BuildComponent,messenger,group chat messenger,))
$(eval $(call BuildComponent,reclaim,reclaim identity-provider subsystem,))
$(eval $(call BuildComponent,rest,REST interface,))
$(eval $(call BuildComponent,rps,RPS routing component,y))
$(eval $(call BuildComponent,namestore-fcfsd,first-come-first-serve registration server,))
$(eval $(call BuildComponent,dhtcache-heap,heap-based dhtcache plugin,y))
$(eval $(call BuildComponent,fs-heap,heap-based filesharing plugin,))
$(eval $(call BuildComponent,gns-flat,flat storage GNS plugins,y))
$(eval $(call BuildComponent,peerstore-flat,flat storage peerstore plugin,))
$(eval $(call BuildComponent,mysql,mySQL backend,))
$(eval $(call BuildComponent,fs-mysql,mySQL filesharing plugins,))
$(eval $(call BuildComponent,pgsql,PostgreSQL backend,))
$(eval $(call BuildComponent,dhtcache-pgsql,PostgreSQL dhtcache plugin,))
$(eval $(call BuildComponent,fs-pgsql,PostgreSQL filesharing plugin,))
$(eval $(call BuildComponent,gns-pgsql,PostgreSQL GNS plugins,))
$(eval $(call BuildComponent,sqlite,libsqlite3 backend,))
$(eval $(call BuildComponent,dhtcache-sqlite,libsqlite3 dhtcache plugin,))
$(eval $(call BuildComponent,fs-sqlite,libsqlite3 filesharing plugin,))
$(eval $(call BuildComponent,gns-sqlite,libsqlite3 gns plugins,))
$(eval $(call BuildComponent,peerstore-sqlite,libsqlite3 peerstore plugin,))
$(eval $(call BuildComponent,transport-bluetooth,bluetooth transport,))
$(eval $(call BuildComponent,transport-http_client,HTTP/HTTPS client transport,y))
$(eval $(call BuildComponent,transport-http_server,HTTP/HTTPS server transport,))
$(eval $(call BuildComponent,transport-tcp,TCP transport,y))
$(eval $(call BuildComponent,transport-udp,UDP transport,y))
$(eval $(call BuildComponent,transport-wlan,WLAN transport,y))
$(eval $(call BuildComponent,utils,administration utililties,))
$(eval $(call BuildComponent,vpn,vpn components,y))
