#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=collectd
PKG_VERSION:=5.12.0
PKG_RELEASE:=9

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://collectd.org/files/ \
	https://github.com/collectd/collectd/releases/download/collectd-$(PKG_VERSION)
PKG_HASH:=5bae043042c19c31f77eb8464e56a01a5454e0b39fa07cf7ad0f1bfc9c3a09d6

PKG_FIXUP:=autoreconf
PKG_REMOVE_FILES:=aclocal.m4 libltdl/aclocal.m4
PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>, Hannu Nyman <hannu.nyman@iki.fi>
PKG_CPE_ID:=cpe:/a:collectd:collectd

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

PKG_CONFIG_DEPENDS:= \
	PACKAGE_COLLECTD_ENCRYPTED_NETWORK \
	PACKAGE_COLLECTD_DEBUG_OUTPUT_ENABLE

COLLECTD_PLUGINS_DISABLED:= \
	amqp \
	ampq1 \
	apple_sensors \
	aquaero \
	barometer \
	battery \
	buddyinfo \
	capabilities \
	ceph \
	cgroups \
	cpusleep \
	curl_jolokia \
	curl_json \
	curl_xml \
	dbi \
	dcpmm \
	dpdkevents \
	dpdkstat \
	dpdk_telemetry \
	drbd \
	fhcount \
	genericjmx \
	gmond \
	gps \
	gpu_nvidia \
	grpc \
	hddtemp \
	hugepages \
	infiniband \
	intel_pmu \
	intel_rdt \
	ipc \
	ipmi \
	ipstats \
	ipvs \
	java \
	logparser \
	log_logstash \
	lvm \
	lpar \
	madwifi \
	mbmon \
	mcelog \
	md \
	mdevents \
	memcachec \
	memcached \
	mic \
	monitorus \
	multimeter \
	netapp \
	netstat_udp \
	nfs \
	notify_desktop \
	notify_email \
	notify_nagios \
	numa \
	onewire \
	openldap \
	openvz \
	oracle \
	ovs_events \
	ovs_stats \
	pcie_errors \
	perl \
	pf \
	pinba \
	procevent \
	python \
	redfish \
	redis \
	rrdcached \
	serial \
	sigrok \
	slurm \
	snmp_agent \
	statsd \
	synproxy \
	sysevent \
	tape \
	tokyotyrant \
	turbostat \
	uuid \
	varnish \
	virt \
	vserver \
	write_influxdb_udp \
	write_kafka \
	write_log \
	write_mongodb \
	write_prometheus \
	write_redis \
	write_riemann \
	write_sensu \
	write_stackdriver \
	write_syslog \
	write_tsdb \
	xencpu \
	xmms \
	zfs_arc \
	zone \
	zookeeper

COLLECTD_PLUGINS_SELECTED:= \
	apache \
	apcups \
	ascent \
	bind \
	chrony \
	conntrack \
	contextswitch \
	cpu \
	cpufreq \
	csv \
	curl \
	df \
	dhcpleases \
	disk \
	dns \
	email \
	entropy \
	ethstat \
	exec \
	filecount \
	fscache \
	interface \
	ipstatistics \
	iptables \
	irq \
	iwinfo \
	load \
	logfile \
	lua \
	match_empty_counter \
	match_hashed \
	match_regex \
	match_timediff \
	match_value \
	memory \
	modbus \
	mqtt \
	mysql \
	netlink \
	network \
	nginx \
	ntpd \
	nut \
	olsrd \
	openvpn \
	ping \
	postgresql \
	powerdns \
	processes \
	protocols \
	routeros \
	rrdtool \
	sensors \
	smart \
	snmp \
	snmp6 \
	swap \
	syslog \
	table \
	tail \
	tail_csv \
	tcpconns \
	teamspeak2 \
	ted \
	thermal \
	threshold \
	ubi \
	unixsock \
	uptime \
	users \
	vmem \
	wireless \
	write_graphite \
	write_http

PKG_CONFIG_DEPENDS:= \
	$(patsubst %,CONFIG_PACKAGE_collectd-mod-%,$(subst _,-,$(COLLECTD_PLUGINS_SELECTED))) \

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk
# collectd-mod-mysql needs iconv
include $(INCLUDE_DIR)/nls.mk

define Package/collectd/Default
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Lightweight system statistics collection daemon
  URL:=https://collectd.org/
endef

define Package/collectd
$(call Package/collectd/Default)
  DEPENDS:= +libpthread \
	  +zlib \
	  +libltdl \
	  +libip4tc \
	  +jshn \
	  +PACKAGE_collectd-mod-lua:liblua
  MENU:=1
endef

define Package/collectd/description
 collectd is a small daemon which collects system information periodically
 and provides mechanismns to store the values in a variety of ways.
endef

define Package/collectd/config
	config PACKAGE_COLLECTD_ENCRYPTED_NETWORK
	bool "Enable ability to use encrypted networking"
	default n
	depends on PACKAGE_collectd
	select PACKAGE_collectd-mod-network

	config PACKAGE_COLLECTD_DEBUG_OUTPUT_ENABLE
	bool "Enable debug output"
	default n
	depends on PACKAGE_collectd
	help
	  Enables the compiler option that collectd is compiled with
	  debugging support. This is used at development stages to get
	  more messages from the collectd during development.
endef

# common configure args
CONFIGURE_ARGS+= \
	--disable-werror \
	--enable-daemon \
	--with-nan-emulation \
	--with-libyajl=no \
	--without-perl-bindings

CONFIGURE_VARS+= \
	CFLAGS="$$$$CFLAGS $(FPIC)" \
	LDFLAGS="$$$$LDFLAGS -lm -lz" \
	KERNEL_DIR="$(LINUX_DIR)"

ifneq ($(CONFIG_PACKAGE_collectd-mod-lua),)
CONFIGURE_VARS+= \
	LDFLAGS="$$$$LDFLAGS -llua"
endif

ifneq ($(CONFIG_PACKAGE_COLLECTD_ENCRYPTED_NETWORK),)
CONFIGURE_ARGS+= \
	--with-libgcrypt=$(STAGING_DIR)/usr
CONFIGURE_VARS+= \
	GCRYPT_LIBS="-lgcrypt"
else
CONFIGURE_ARGS+= \
	--without-libgcrypt
endif

ifeq ($(CONFIG_PACKAGE_COLLECTD_DEBUG_OUTPUT_ENABLE),y)
CONFIGURE_ARGS+= \
	--enable-debug
else
CONFIGURE_ARGS+= \
	--disable-debug
endif

CONFIGURE_PLUGIN= \
	$(foreach m, $(1), \
		$(if $(CONFIG_PACKAGE_collectd-mod-$(subst _,-,$(m))),--enable-$(m),--disable-$(m)) \
	)

CONFIGURE_ARGS+= \
	$(call CONFIGURE_PLUGIN,$(COLLECTD_PLUGINS_SELECTED)) \
	$(call CONFIGURE_PLUGIN,$(COLLECTD_PLUGINS_DISABLED)) \

# exception: mod-ascent needs libxml2
ifneq ($(CONFIG_PACKAGE_collectd-mod-ascent),)
  CONFIGURE_VARS+= \
	CPPFLAGS="$$$$CPPFLAGS -I$(STAGING_DIR)/usr/include/libxml2"
endif

ifneq ($(CONFIG_BIG_ENDIAN),)
  CONFIGURE_ARGS+= --with-fp-layout=endianflip
else
  CONFIGURE_ARGS+= --with-fp-layout=nothing
endif

ifneq ($(CONFIG_PACKAGE_collectd-mod-postgresql),)
  CONFIGURE_ARGS+= --with-libpq="$(STAGING_DIR)/usr/"
endif

ifneq ($(CONFIG_PACKAGE_collectd-mod-mysql),)
  CONFIGURE_ARGS+= --with-libmysql="$(STAGING_DIR)/usr/"
endif

# exception: mod-netlink needs libnetlink from iproute
ifneq ($(CONFIG_PACKAGE_collectd-mod-netlink),)
  CONFIGURE_ARGS+= --with-libnetlink="$(STAGING_DIR)/usr"
endif

# exception: mod-modbus needs libmodbus
ifneq ($(CONFIG_PACKAGE_collectd-mod-modbus),)
  CONFIGURE_ARGS+= --with-libmodbus="$(STAGING_DIR)/usr"
endif

# exception: mod-rrdtool needs rrdtool-1.0.x
ifneq ($(CONFIG_PACKAGE_collectd-mod-rrdtool),)
  CONFIGURE_ARGS+= --with-librrd="$(STAGING_DIR)/usr/lib/rrdtool-1.0"
endif

# exception: mod-smart needs libatasmart
ifneq ($(CONFIG_PACKAGE_collectd-mod-smart),)
  CONFIGURE_ARGS+= \
	--with-libatasmart="$(STAGING_DIR)/usr" \
	--with-libudev="$(STAGING_DIR)/usr"
else
  CONFIGURE_ARGS+= --without-libudev
endif

define Package/collectd/conffiles
/etc/collectd.conf
/etc/config/collectd
endef

define Package/collectd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/collectd $(1)/usr/sbin/

	$(INSTALL_DIR) $(1)/usr/share/collectd
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/collectd/types.db $(1)/usr/share/collectd/

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/collectd.init $(1)/etc/init.d/collectd

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) files/collectd.uci $(1)/etc/config/collectd
endef

# 1: plugin name
# 2: plugin title/description
# 3: files
# 4: extra dependency
define BuildPlugin

  PKG_CONFIG_DEPENDS+= CONFIG_PACKAGE_collectd-mod-$(1)

  define Package/collectd-mod-$(1)
  $$(call Package/collectd/Default)
    TITLE:=$(2) plugin
    DEPENDS:= collectd $(4)
  endef

  define Package/collectd-mod-$(1)/install
	$(INSTALL_DIR) $$(1)/usr/lib/collectd
	for m in $(3); do \
		$(CP) \
			$(PKG_INSTALL_DIR)/usr/lib/collectd/$$$$$$$${m}.so \
			$$(1)/usr/lib/collectd/ ; \
	done
	if [ -f ./files/usr/share/collectd/plugin/$(1).json ]; then \
		$(INSTALL_DIR) $$(1)/usr/share/collectd/plugin ; \
		$(INSTALL_DATA) ./files/usr/share/collectd/plugin/$(1).json \
			$$(1)/usr/share/collectd/plugin/$(1).json ; \
	fi
  endef

  $$(eval $$(call BuildPackage,collectd-mod-$(1)))

endef

define BuildScriptPlugin
  PKG_CONFIG_DEPENDS+= CONFIG_PACKAGE_collectd-mod-$(1)

  define Package/collectd-mod-$(1)
  $$(call Package/collectd/Default)
    TITLE:=$(2) plugin
    DEPENDS:= $(4)
  endef

  define Package/collectd-mod-$(1)/install
	$(INSTALL_DIR) $$(1)/usr/libexec/collectd
	for m in $(3); do \
		${INSTALL_BIN} \
			./files/exec-scripts/$$$$$$$${m}.sh $$(1)/usr/libexec/collectd/ ; \
	done
	if [ -f ./files/usr/share/collectd/plugin/$(1).json ]; then \
		$(INSTALL_DIR) $$(1)/usr/share/collectd/plugin ; \
		$(INSTALL_DATA) ./files/usr/share/collectd/plugin/$(1).json \
			$$(1)/usr/share/collectd/plugin/$(1).json ; \
	fi
  endef

  $$(eval $$(call BuildPackage,collectd-mod-$(1)))

endef

$(eval $(call BuildPackage,collectd))

#$(eval $(call BuildPlugin,NAME,DESCRIPTION,FILES,DEPENDENCIES))
$(eval $(call BuildPlugin,apache,apache status input,apache,+PACKAGE_collectd-mod-apache:libcurl))
$(eval $(call BuildPlugin,apcups,apcups status input,apcups,))
$(eval $(call BuildPlugin,ascent,ascent status input,ascent,+PACKAGE_collectd-mod-ascent:libcurl +PACKAGE_collectd-mod-ascent:libxml2))
$(eval $(call BuildPlugin,bind,BIND server/zone input,bind,+PACKAGE_collectd-mod-bind:libcurl +PACKAGE_collectd-mod-bind:libxml2))
$(eval $(call BuildPlugin,chrony,chrony status input,chrony,))
$(eval $(call BuildPlugin,conntrack,connection tracking table size input,conntrack,))
$(eval $(call BuildPlugin,contextswitch,context switch input,contextswitch,))
$(eval $(call BuildPlugin,cpu,CPU input,cpu,))
$(eval $(call BuildPlugin,cpufreq,CPU Freq input,cpufreq,@(TARGET_x86||TARGET_x86_64||TARGET_mvebu||TARGET_ipq806x||TARGET_armvirt||TARGET_ipq40xx||TARGET_bcm27xx_bcm2709||TARGET_rockchip))) # Only enable on targets with CPUs supporting frequency scaling
$(eval $(call BuildPlugin,csv,CSV output,csv,))
$(eval $(call BuildPlugin,curl,cURL input,curl,+PACKAGE_collectd-mod-curl:libcurl))
#$(eval $(call BuildPlugin,dbi,relational database input,dbi,+PACKAGE_collectd-mod-dbi:libdbi))
$(eval $(call BuildPlugin,df,disk space input,df,))
$(eval $(call BuildPlugin,dhcpleases,show dhcpleases,dhcpleases,))
$(eval $(call BuildPlugin,disk,disk usage/timing input,disk,+PACKAGE_collectd-mod-smart:libudev))
$(eval $(call BuildPlugin,dns,DNS traffic input,dns,+PACKAGE_collectd-mod-dns:libpcap))
$(eval $(call BuildPlugin,email,email output,email,))
$(eval $(call BuildPlugin,entropy,Entropy amount input,entropy,))
$(eval $(call BuildPlugin,ethstat,Ethernet adapter statistics input,ethstat,))
$(eval $(call BuildPlugin,exec,process exec input,exec,))
$(eval $(call BuildPlugin,filecount,file count input,filecount,))
$(eval $(call BuildPlugin,fscache,file-system based caching framework input,fscache,))
$(eval $(call BuildPlugin,interface,network interfaces input,interface,))
$(eval $(call BuildPlugin,ipstatistics,ipstatistics input,ipstatistics,))
$(eval $(call BuildPlugin,iptables,iptables status input,iptables,+PACKAGE_collectd-mod-iptables:iptables +libip4tc +libip6tc))
$(eval $(call BuildPlugin,irq,interrupt usage input,irq,))
$(eval $(call BuildPlugin,iwinfo,libiwinfo wireless statistics,iwinfo,+PACKAGE_collectd-mod-iwinfo:libiwinfo))
$(eval $(call BuildPlugin,load,system load input,load,))
$(eval $(call BuildPlugin,logfile,log files output,logfile,))
$(eval $(call BuildPlugin,lua,lua input/output,lua,+PACKAGE_collectd-mod-lua:liblua))
$(eval $(call BuildPlugin,match-empty-counter,empty-counter match,match_empty_counter,))
$(eval $(call BuildPlugin,match-hashed,hashed match,match_hashed,))
$(eval $(call BuildPlugin,match-regex,regex match,match_regex,))
$(eval $(call BuildPlugin,match-timediff,timediff match,match_timediff,))
$(eval $(call BuildPlugin,match-value,value match,match_value,))
$(eval $(call BuildPlugin,mysql,MySQL status input,mysql,+PACKAGE_collectd-mod-mysql:libmysqlclient-r))
$(eval $(call BuildPlugin,memory,physical memory usage input,memory,))
$(eval $(call BuildPlugin,modbus,read variables through libmodbus,modbus,+PACKAGE_collectd-mod-modbus:libmodbus))
$(eval $(call BuildPlugin,mqtt,transmit data with MQTT,mqtt,+PACKAGE_collectd-mod-mqtt:libmosquitto))
$(eval $(call BuildPlugin,netlink,netlink input,netlink,+PACKAGE_collectd-mod-netlink:libmnl))
$(eval $(call BuildPlugin,network,network input/output,network,+PACKAGE_COLLECTD_ENCRYPTED_NETWORK:libgcrypt))
$(eval $(call BuildPlugin,nginx,nginx status input,nginx,+PACKAGE_collectd-mod-nginx:libcurl))
$(eval $(call BuildPlugin,ntpd,NTP daemon status input,ntpd,))
$(eval $(call BuildPlugin,nut,UPS monitoring input,nut,+PACKAGE_collectd-mod-nut:nut-common))
$(eval $(call BuildPlugin,olsrd,OLSRd status input,olsrd,))
$(eval $(call BuildPlugin,openvpn,OpenVPN traffic/compression input,openvpn,))
$(eval $(call BuildPlugin,ping,ping status input,ping,+PACKAGE_collectd-mod-ping:liboping))
$(eval $(call BuildPlugin,postgresql,PostgreSQL status input,postgresql,+PACKAGE_collectd-mod-postgresql:libpq))
$(eval $(call BuildPlugin,powerdns,PowerDNS server status input,powerdns,))
$(eval $(call BuildPlugin,processes,process status input,processes,+PACKAGE_collectd-mod-processes:libmnl))
$(eval $(call BuildPlugin,protocols,network protocols input,protocols,))
$(eval $(call BuildPlugin,routeros,MikroTik RouterOS input,routeros,+PACKAGE_collectd-mod-routeros:librouteros))
$(eval $(call BuildPlugin,rrdtool,RRDtool output,rrdtool,+PACKAGE_collectd-mod-rrdtool:librrd1))
$(eval $(call BuildPlugin,sensors,lm_sensors input,sensors,+PACKAGE_collectd-mod-sensors:libsensors))
$(eval $(call BuildPlugin,smart,smart input,smart,+PACKAGE_collectd-mod-smart:libatasmart))
$(eval $(call BuildPlugin,snmp,SNMP input,snmp,+PACKAGE_collectd-mod-snmp:libnetsnmp))
$(eval $(call BuildPlugin,snmp6,snmp6 input,snmp6,))
$(eval $(call BuildPlugin,swap,swap input,swap,))
$(eval $(call BuildPlugin,syslog,syslog output,syslog,))
$(eval $(call BuildPlugin,tail,tail input,tail,))
$(eval $(call BuildPlugin,tail-csv,tail CSV input,tail_csv,))
$(eval $(call BuildPlugin,table,table-like structured file input,table,))
$(eval $(call BuildPlugin,teamspeak2,TeamSpeak2 input,teamspeak2,))
$(eval $(call BuildPlugin,ted,The Energy Detective input,ted,))
$(eval $(call BuildPlugin,tcpconns,TCP connection tracking input,tcpconns,))
$(eval $(call BuildPlugin,thermal,system temperatures input,thermal,))
$(eval $(call BuildPlugin,threshold,Notifications and thresholds,threshold,))
$(eval $(call BuildPlugin,ubi,Unsorted block images,ubi,@NAND_SUPPORT))
$(eval $(call BuildPlugin,unixsock,unix socket output,unixsock,))
$(eval $(call BuildPlugin,uptime,uptime status input,uptime,))
$(eval $(call BuildPlugin,users,user logged in status input,users,))
$(eval $(call BuildPlugin,vmem,virtual memory usage input,vmem,))
$(eval $(call BuildPlugin,wireless,wireless status input,wireless,))
$(eval $(call BuildPlugin,write-graphite,Carbon/Graphite output,write_graphite,+PACKAGE_collectd-mod-write-graphite:libpthread))
$(eval $(call BuildPlugin,write-http,HTTP POST output,write_http,+PACKAGE_collectd-mod-write-http:libcurl))

$(eval $(call BuildScriptPlugin,sqm,SQM/qdisc collection,sqm_collectd,+PACKAGE_collectd-mod-sqm:collectd-mod-exec))
