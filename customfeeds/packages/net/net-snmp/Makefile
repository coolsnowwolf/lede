#
# Copyright (C) 2006-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=net-snmp
PKG_VERSION:=5.9.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/net-snmp
PKG_HASH:=eb7fd4a44de6cddbffd9a92a85ad1309e5c1054fb9d5a7dd93079c8953f48c3f
PKG_MAINTAINER:=Stijn Tintel <stijn@linux-ipv6.be>
PKG_LICENSE:=MIT BSD-3-Clause-Clear
PKG_CPE_ID:=cpe:/a:net-snmp:net-snmp

PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

define Package/net-snmp/Default
  SECTION:=net
  CATEGORY:=Network
  URL:=http://www.net-snmp.org/
endef

define Package/net-snmp/Default/description
 Simple Network Management Protocol (SNMP) is a widely used protocol for
 monitoring the health and welfare of network equipment (eg. routers),
 computer equipment and even devices like UPSs. Net-SNMP is a suite of
 applications used to implement SNMP v1, SNMP v2c and SNMP v3 using both
 IPv4 and IPv6.
endef


define Package/libnetsnmp
$(call Package/net-snmp/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libnl-tiny +libpci +libpcre
  TITLE:=Open source SNMP implementation (libraries)
endef

define Package/libnetsnmp/description
$(call Package/net-snmp/Default/description)
 .
 This package contains shared libraries, needed by other programs.
endef


define Package/snmp-mibs
$(call Package/net-snmp/Default)
  TITLE:=Open source SNMP implementation (MIB-files)
endef

define Package/snmp-mibs/description
$(call Package/net-snmp/Default/description)
 .
 This package contains SNMP MIB-Files.
endef


define Package/snmp-utils
$(call Package/net-snmp/Default)
  DEPENDS:=+libnetsnmp
  TITLE:=Open source SNMP implementation (utilities)
endef

define Package/snmp-utils/description
$(call Package/net-snmp/Default/description)
 .
 This package contains SNMP client utilities:
   - snmpget
   - snmpset
   - snmpstatus
   - snmptest
   - snmptrap
   - snmpwalk
endef


define Package/snmpd
$(call Package/net-snmp/Default)
  DEPENDS:=+libnetsnmp
  TITLE:=Open source SNMP implementation (daemon)
endef

define Package/snmpd/description
$(call Package/net-snmp/Default/description)
 .
 This package contains the SNMP agent, dynamically linked.
endef


define Package/snmpd-static
$(call Package/net-snmp/Default)
  DEPENDS:=+snmpd
  TITLE:=Open source SNMP implementation (daemon)
  BUILDONLY:=1
endef


define Package/snmptrapd
$(call Package/net-snmp/Default)
  DEPENDS:=+libnetsnmp
  TITLE:=Open source SNMP implementation (notification receiver)
endef

define Package/snmptrapd/description
$(call Package/net-snmp/Default/description)
 .
 This package contains the SNMP notification receiver.
endef


SNMP_MIB_MODULES_INCLUDED = \
	agent/extend \
	agentx \
	host/hr_device \
	host/hr_disk \
	host/hr_filesys \
	host/hr_network \
	host/hr_partition \
	host/hr_proc \
	host/hr_storage \
	host/hr_system \
	ieee802dot11 \
	if-mib/ifXTable \
	ip-mib/inetNetToMediaTable \
	mibII/at \
	mibII/icmp \
	mibII/ifTable \
	mibII/ip \
	mibII/snmp_mib \
	mibII/sysORTable \
	mibII/system_mib \
	mibII/tcp \
	mibII/udp \
	mibII/vacm_context \
	mibII/vacm_vars \
	snmpv3/snmpEngine \
	snmpv3/snmpMPDStats \
	snmpv3/usmConf \
	snmpv3/usmStats \
	snmpv3/usmUser \
	tunnel \
	ucd-snmp/disk \
	ucd-snmp/dlmod \
	ucd-snmp/extensible \
	ucd-snmp/loadave \
	ucd-snmp/memory \
	ucd-snmp/pass \
	ucd-snmp/pass_persist \
	ucd-snmp/proc \
	ucd-snmp/vmstat \
	util_funcs \
	utilities/execute \

SNMP_MIB_MODULES_EXCLUDED = \
	agent_mibs \
	disman/event \
	disman/schedule \
	hardware \
	host \
	if-mib \
	ip-mib \
	mibII \
	notification \
	notification-log-mib \
	snmpv3mibs \
	target \
	tcp-mib \
	ucd_snmp \
	udp-mib \
	utilities \

SNMP_TRANSPORTS_INCLUDED = Callback UDP Unix

SNMP_TRANSPORTS_EXCLUDED = TCP TCPIPv6

TARGET_CFLAGS += $(FPIC)
TARGET_CPPFLAGS += -I$(STAGING_DIR)/usr/include/libnl-tiny

CONFIGURE_ARGS += \
	--enable-mfd-rewrites \
	--enable-shared \
	--enable-static \
	--with-endianness=$(if $(CONFIG_BIG_ENDIAN),big,little) \
	--with-logfile=/var/log/snmpd.log \
	--with-persistent-directory=/usr/lib/snmp/ \
	--with-default-snmp-version=1 \
	--with-sys-contact=root@localhost \
	--with-sys-location=Unknown \
	--enable-applications \
	--disable-debugging \
	--disable-manuals \
	--disable-scripts \
	--with-out-mib-modules="$(SNMP_MIB_MODULES_EXCLUDED)" \
	--with-mib-modules="$(SNMP_MIB_MODULES_INCLUDED)" \
	--with-out-transports="$(SNMP_TRANSPORTS_EXCLUDED)" \
	--with-transports="$(SNMP_TRANSPORTS_INCLUDED)" \
	--without-openssl \
	--without-libwrap \
	--without-mysql \
	--without-rpm \
	--without-zlib \
	--with-nl \
	 $(call autoconf_bool,CONFIG_IPV6,ipv6) \
	--disable-perl-cc-checks \
	--disable-embedded-perl \
	--without-perl-modules

CONFIGURE_VARS += \
	ac_cv_header_netlink_netlink_h=yes \
	netsnmp_cv_func_nl_connect_LIBS=-lnl-tiny \

ifeq ($(CONFIG_IPV6),y)
SNMP_TRANSPORTS_INCLUDED+= UDPIPv6
endif

TARGET_LDFLAGS += -L$(TOOLCHAIN_DIR)/usr/lib

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		LDFLAGS="$(TARGET_LDFLAGS) -lm -lc" \
		all install
endef

define Build/InstallDev
	$(INSTALL_DIR) $(2)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/net-snmp-config $(2)/bin/
	$(SED) 's,=/usr,=$(STAGING_DIR)/usr,g' $(2)/bin/net-snmp-config
	$(INSTALL_DIR) $(STAGING_DIR)/usr/bin
	$(LN) $(STAGING_DIR)/host/bin/net-snmp-config $(STAGING_DIR)/usr/bin/

	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/net-snmp $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnetsnmp{,agent,helpers,mibs}.{a,so*} $(1)/usr/lib/
endef

define Package/libnetsnmp/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnetsnmp{,agent,helpers,mibs}.so.* $(1)/usr/lib/
endef

define Package/snmp-mibs/install
	$(INSTALL_DIR) $(1)/usr/share/snmp/mibs
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/snmp/mibs/* $(1)/usr/share/snmp/mibs/
endef

define Package/snmp-utils/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/snmp{get,set,status,test,trap,walk} $(1)/usr/bin/
endef

define Package/snmpd/conffiles
/etc/config/snmpd
endef

define Package/snmpd/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/snmpd.conf $(1)/etc/config/snmpd
	$(INSTALL_DIR) $(1)/etc/snmp
	$(LN) /var/run/snmpd.conf $(1)/etc/snmp/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/snmpd.init $(1)/etc/init.d/snmpd
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/snmpd $(1)/usr/sbin/snmpd
endef

define Package/snmptrapd/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/snmptrapd.init $(1)/etc/init.d/snmptrapd
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnetsnmptrapd.so.* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/snmptrapd $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,libnetsnmp))
$(eval $(call BuildPackage,snmp-mibs))
$(eval $(call BuildPackage,snmp-utils))
$(eval $(call BuildPackage,snmpd))
$(eval $(call BuildPackage,snmpd-static))
$(eval $(call BuildPackage,snmptrapd))
