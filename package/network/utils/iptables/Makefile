#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=iptables
PKG_VERSION:=1.8.7
PKG_RELEASE:=2

PKG_SOURCE_URL:=https://netfilter.org/projects/iptables/files
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_HASH:=c109c96bb04998cd44156622d36f8e04b140701ec60531a10668cfdff5e8d8f0

PKG_FIXUP:=autoreconf
PKG_FLAGS:=nonshared

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_LICENSE:=GPL-2.0
PKG_CPE_ID:=cpe:/a:netfilter_core_team:iptables

include $(INCLUDE_DIR)/package.mk
ifeq ($(DUMP),)
  -include $(LINUX_DIR)/.config
  include $(INCLUDE_DIR)/netfilter.mk
  STAMP_CONFIGURED:=$(strip $(STAMP_CONFIGURED))_$(shell grep 'NETFILTER' $(LINUX_DIR)/.config | $(MKHASH) md5)
endif


define Package/iptables/Default
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Firewall
  URL:=https://netfilter.org/
endef

define Package/iptables/Module
$(call Package/iptables/Default)
  DEPENDS:=iptables $(1)
endef

define Package/iptables
$(call Package/iptables/Default)
  TITLE:=IP firewall administration tool
  MENU:=1
  DEPENDS+= +kmod-ipt-core +libip4tc +IPV6:libip6tc +libxtables
endef

define Package/iptables/config
  config IPTABLES_CONNLABEL
	bool "Enable Connlabel support"
	default n
	help
		This enable connlabel support in iptables.

  config IPTABLES_NFTABLES
	bool "Enable Nftables support"
	default n
	help
		This enable nftables support in iptables.
endef

define Package/iptables/description
IP firewall administration tool.

 Matches:
  - icmp
  - tcp
  - udp
  - comment
  - conntrack
  - limit
  - mac
  - mark
  - multiport
  - set
  - state
  - time

 Targets:
  - ACCEPT
  - CT
  - DNAT
  - DROP
  - REJECT
  - FLOWOFFLOAD
  - LOG
  - MARK
  - MASQUERADE
  - REDIRECT
  - SET
  - SNAT
  - TCPMSS

 Tables:
  - filter
  - mangle
  - nat
  - raw

endef

define Package/iptables-nft
$(call Package/iptables/Default)
  TITLE:=IP firewall administration tool nft
  DEPENDS:=iptables @IPTABLES_NFTABLES +libxtables-nft
endef

define Package/iptables-nft/description
Extra iptables nftables nft binaries.
  iptables-nft
  iptables-nft-restore
  iptables-nft-save
  iptables-translate
  iptables-restore-translate
endef

define Package/iptables-mod-conntrack-extra
$(call Package/iptables/Module, +kmod-ipt-conntrack-extra +kmod-ipt-raw)
  TITLE:=Extra connection tracking extensions
endef

define Package/iptables-mod-conntrack-extra/description
Extra iptables extensions for connection tracking.

 Matches:
  - connbytes
  - connlimit
  - connmark
  - recent
  - helper

 Targets:
  - CONNMARK

endef

define Package/iptables-mod-conntrack-label
$(call Package/iptables/Module, +kmod-ipt-conntrack-label @IPTABLES_CONNLABEL)
  TITLE:=Connection tracking labeling extension
  DEFAULT:=y if IPTABLES_CONNLABEL
endef

define Package/iptables-mod-conntrack-label/description
Match and set label(s) on connection tracking entries

 Matches:
  - connlabel

endef

define Package/iptables-mod-filter
$(call Package/iptables/Module, +kmod-ipt-filter)
  TITLE:=Content inspection extensions
endef

define Package/iptables-mod-filter/description
iptables extensions for packet content inspection.
Includes support for:

 Matches:
  - string
  - bpf

endef

define Package/iptables-mod-ipopt
$(call Package/iptables/Module, +kmod-ipt-ipopt)
  TITLE:=IP/Packet option extensions
endef

define Package/iptables-mod-ipopt/description
iptables extensions for matching/changing IP packet options.

 Matches:
  - dscp
  - ecn
  - length
  - statistic
  - tcpmss
  - unclean
  - hl

 Targets:
  - DSCP
  - CLASSIFY
  - ECN
  - HL

endef

define Package/iptables-mod-ipsec
$(call Package/iptables/Module, +kmod-ipt-ipsec)
  TITLE:=IPsec extensions
endef

define Package/iptables-mod-ipsec/description
iptables extensions for matching ipsec traffic.

 Matches:
  - ah
  - esp
  - policy

endef

define Package/iptables-mod-nat-extra
$(call Package/iptables/Module, +kmod-ipt-nat-extra)
  TITLE:=Extra NAT extensions
endef

define Package/iptables-mod-nat-extra/description
iptables extensions for extra NAT targets.

 Targets:
  - MIRROR
  - NETMAP
endef

define Package/iptables-mod-nflog
$(call Package/iptables/Module, +kmod-nfnetlink-log +kmod-ipt-nflog)
  TITLE:=Netfilter NFLOG target
endef

define Package/iptables-mod-nflog/description
 iptables extension for user-space logging via NFNETLINK.

 Includes:
  - libxt_NFLOG

endef

define Package/iptables-mod-trace
$(call Package/iptables/Module, +kmod-ipt-debug)
  TITLE:=Netfilter TRACE target
endef

define Package/iptables-mod-trace/description
 iptables extension for TRACE target

 Includes:
  - libxt_TRACE

endef


define Package/iptables-mod-nfqueue
$(call Package/iptables/Module, +kmod-nfnetlink-queue +kmod-ipt-nfqueue)
  TITLE:=Netfilter NFQUEUE target
endef

define Package/iptables-mod-nfqueue/description
 iptables extension for user-space queuing via NFNETLINK.

 Includes:
  - libxt_NFQUEUE

endef

define Package/iptables-mod-hashlimit
$(call Package/iptables/Module, +kmod-ipt-hashlimit)
  TITLE:=hashlimit matching
endef

define Package/iptables-mod-hashlimit/description
iptables extensions for hashlimit matching

 Matches:
  - hashlimit

endef

define Package/iptables-mod-rpfilter
$(call Package/iptables/Module, +kmod-ipt-rpfilter)
  TITLE:=rpfilter iptables extension
endef

define Package/iptables-mod-rpfilter/description
iptables extensions for reverse path filter test on a packet

 Matches:
  - rpfilter

endef

define Package/iptables-mod-iprange
$(call Package/iptables/Module, +kmod-ipt-iprange)
  TITLE:=IP range extension
endef

define Package/iptables-mod-iprange/description
iptables extensions for matching ip ranges.

 Matches:
  - iprange

endef

define Package/iptables-mod-cluster
$(call Package/iptables/Module, +kmod-ipt-cluster)
  TITLE:=Match cluster extension
endef

define Package/iptables-mod-cluster/description
iptables extensions for matching cluster.

 Netfilter (IPv4/IPv6) module for matching cluster
 This option allows you to build work-load-sharing clusters of
 network servers/stateful firewalls without having a dedicated
 load-balancing router/server/switch. Basically, this match returns
 true when the packet must be handled by this cluster node. Thus,
 all nodes see all packets and this match decides which node handles
 what packets. The work-load sharing algorithm is based on source
 address hashing.

 This module is usable for ipv4 and ipv6.

 If you select it, it enables kmod-ipt-cluster.

 see `iptables -m cluster --help` for more information.
endef

define Package/iptables-mod-clusterip
$(call Package/iptables/Module, +kmod-ipt-clusterip)
  TITLE:=Clusterip extension
endef

define Package/iptables-mod-clusterip/description
iptables extensions for CLUSTERIP.
 The CLUSTERIP target allows you to build load-balancing clusters of
 network servers without having a dedicated load-balancing
 router/server/switch.

 If you select it, it enables kmod-ipt-clusterip.

 see `iptables -j CLUSTERIP --help` for more information.
endef

define Package/iptables-mod-extra
$(call Package/iptables/Module, +kmod-ipt-extra)
  TITLE:=Other extra iptables extensions
endef

define Package/iptables-mod-extra/description
Other extra iptables extensions.

 Matches:
  - addrtype
  - condition
  - owner
  - pkttype
  - quota

endef

define Package/iptables-mod-physdev
$(call Package/iptables/Module, +kmod-ipt-physdev)
  TITLE:=physdev iptables extension
endef

define Package/iptables-mod-physdev/description
The iptables physdev match.
endef

define Package/iptables-mod-led
$(call Package/iptables/Module, +kmod-ipt-led)
  TITLE:=LED trigger iptables extension
endef

define Package/iptables-mod-led/description
iptables extension for triggering a LED.

 Targets:
  - LED

endef

define Package/iptables-mod-socket
$(call Package/iptables/Module, +kmod-ipt-socket)
  TITLE:=Socket match iptables extensions
endef

define Package/iptables-mod-socket/description
Socket match iptables extensions.

 Matches:
  - socket

endef

define Package/iptables-mod-tproxy
$(call Package/iptables/Module, +kmod-ipt-tproxy)
  TITLE:=Transparent proxy iptables extensions
endef

define Package/iptables-mod-tproxy/description
Transparent proxy iptables extensions.

 Targets:
  - TPROXY

endef

define Package/iptables-mod-tee
$(call Package/iptables/Module, +kmod-ipt-tee)
  TITLE:=TEE iptables extensions
endef

define Package/iptables-mod-tee/description
TEE iptables extensions.

 Targets:
  - TEE

endef

define Package/iptables-mod-u32
$(call Package/iptables/Module, +kmod-ipt-u32)
  TITLE:=U32 iptables extensions
endef

define Package/iptables-mod-u32/description
U32 iptables extensions.

 Matches:
  - u32

endef

define Package/iptables-mod-checksum
$(call Package/iptables/Module, +kmod-ipt-checksum)
  TITLE:=IP CHECKSUM target extension
endef

define Package/iptables-mod-checksum/description
iptables extension for the CHECKSUM calculation target
endef

define Package/ip6tables
$(call Package/iptables/Default)
  DEPENDS:=@IPV6 +kmod-ip6tables +iptables
  CATEGORY:=Network
  TITLE:=IPv6 firewall administration tool
  MENU:=1
endef

define Package/ip6tables-nft
$(call Package/iptables/Default)
  DEPENDS:=ip6tables @IPTABLES_NFTABLES +libxtables-nft
  TITLE:=IP firewall administration tool nft
endef

define Package/ip6tables-nft/description
Extra ip6tables nftables nft binaries.
  iptables-nft
  iptables-nft-restore
  iptables-nft-save
  iptables-translate
  iptables-restore-translate
endef

define Package/ip6tables-extra
$(call Package/iptables/Default)
  DEPENDS:=ip6tables +kmod-ip6tables-extra
  TITLE:=IPv6 header matching modules
endef

define Package/ip6tables-mod-extra/description
iptables header matching modules for IPv6
endef

define Package/ip6tables-mod-nat
$(call Package/iptables/Default)
  DEPENDS:=ip6tables +kmod-ipt-nat6
  TITLE:=IPv6 NAT extensions
endef

define Package/ip6tables-mod-nat/description
iptables extensions for IPv6-NAT targets.
endef

define Package/libip4tc
$(call Package/iptables/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=IPv4 firewall - shared libiptc library
  ABI_VERSION:=2
  DEPENDS:=+libxtables
endef

define Package/libip6tc
$(call Package/iptables/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=IPv6 firewall - shared libiptc library
  ABI_VERSION:=2
  DEPENDS:=+libxtables
endef

define Package/libxtables
 $(call Package/iptables/Default)
 SECTION:=libs
 CATEGORY:=Libraries
 TITLE:=IPv4/IPv6 firewall - shared xtables library
 ABI_VERSION:=12
 DEPENDS:= \
	+IPTABLES_CONNLABEL:libnetfilter-conntrack \
	+IPTABLES_NFTABLES:libnftnl
endef

define Package/libxtables-nft
 $(call Package/iptables/Default)
 SECTION:=libs
 CATEGORY:=Libraries
 TITLE:=IPv4/IPv6 firewall - shared xtables nft library
 ABI_VERSION:=12
 DEPENDS:=libxtables
endef

TARGET_CPPFLAGS := \
	-I$(PKG_BUILD_DIR)/include \
	-I$(LINUX_DIR)/user_headers/include \
	$(TARGET_CPPFLAGS)

TARGET_CFLAGS += \
	-I$(PKG_BUILD_DIR)/include \
	-I$(LINUX_DIR)/user_headers/include \
	-ffunction-sections -fdata-sections \
	-DNO_LEGACY

TARGET_LDFLAGS += \
	-Wl,--gc-sections

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \
	--enable-devel \
	--with-kernel="$(LINUX_DIR)/user_headers" \
	--with-xtlibdir=/usr/lib/iptables \
	--with-xt-lock-name=/var/run/xtables.lock \
	$(if $(CONFIG_IPTABLES_CONNLABEL),,--disable-connlabel) \
	$(if $(CONFIG_IPTABLES_NFTABLES),,--disable-nftables) \
	$(if $(CONFIG_IPV6),,--disable-ipv6)

MAKE_FLAGS := \
	$(TARGET_CONFIGURE_OPTS) \
	COPT_FLAGS="$(TARGET_CFLAGS)" \
	KERNEL_DIR="$(LINUX_DIR)/user_headers/" PREFIX=/usr \
	KBUILD_OUTPUT="$(LINUX_DIR)" \
	BUILTIN_MODULES="$(patsubst ip6t_%,%,$(patsubst ipt_%,%,$(patsubst xt_%,%,$(IPT_BUILTIN) $(IPT_CONNTRACK-m) $(IPT_NAT-m))))"

ifneq ($(wildcard $(PKG_BUILD_DIR)/.config_*),$(subst .configured_,.config_,$(STAMP_CONFIGURED)))
  define Build/Configure/rebuild
	$(FIND) $(PKG_BUILD_DIR) -name \*.o -or -name \*.\?o -or -name \*.a | $(XARGS) rm -f
	rm -f $(PKG_BUILD_DIR)/.config_*
	rm -f $(PKG_BUILD_DIR)/.configured_*
	touch $(subst .configured_,.config_,$(STAMP_CONFIGURED))
  endef
endif

define Build/Configure
$(Build/Configure/rebuild)
$(Build/Configure/Default)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/include/iptables
	$(INSTALL_DIR) $(1)/usr/include/net/netfilter

	# XXX: iptables header fixup, some headers are not installed by iptables anymore
	$(CP) $(PKG_BUILD_DIR)/include/iptables/*.h $(1)/usr/include/iptables/
	$(CP) $(PKG_BUILD_DIR)/include/iptables.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/include/ip6tables.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/include/libipulog $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/include/libiptc $(1)/usr/include/

	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libxtables.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libip*tc.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/xtables.pc $(1)/usr/lib/pkgconfig/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libip*tc.pc $(1)/usr/lib/pkgconfig/

	# XXX: needed by firewall3
	$(CP) $(PKG_BUILD_DIR)/extensions/libiptext*.so $(1)/usr/lib/
endef

define Package/iptables/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/xtables-legacy-multi $(1)/usr/sbin/
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/iptables{,-restore,-save} $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/usr/lib/iptables
endef

define Package/iptables-nft/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/xtables-nft-multi $(1)/usr/sbin/
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/iptables-nft{,-restore,-save} $(1)/usr/sbin/
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/iptables{,-restore}-translate $(1)/usr/sbin/
endef

define Package/ip6tables/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/ip6tables{,-restore,-save} $(1)/usr/sbin/
endef

define Package/ip6tables-nft/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/ip6tables-nft{,-restore,-save} $(1)/usr/sbin/
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/ip6tables{,-restore}-translate $(1)/usr/sbin/
endef

define Package/libip4tc/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libip4tc.so.* $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/extensions/libiptext4.so $(1)/usr/lib/
endef

define Package/libip6tc/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libip6tc.so.* $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/extensions/libiptext6.so $(1)/usr/lib/
endef

define Package/libxtables/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libxtables.so.* $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/extensions/libiptext.so $(1)/usr/lib/
endef

define Package/libxtables-nft/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/extensions/libiptext_*.so $(1)/usr/lib/
endef

define BuildPlugin
  define Package/$(1)/install
	$(INSTALL_DIR) $$(1)/usr/lib/iptables
	for m in $(patsubst xt_%,ipt_%,$(2)) $(patsubst ipt_%,xt_%,$(2)) $(patsubst xt_%,ip6t_%,$(2)) $(patsubst ip6t_%,xt_%,$(2)); do \
		if [ -f $(PKG_INSTALL_DIR)/usr/lib/iptables/lib$$$$$$$${m}.so ]; then \
			$(CP) $(PKG_INSTALL_DIR)/usr/lib/iptables/lib$$$$$$$${m}.so $$(1)/usr/lib/iptables/ ; \
		fi; \
	done
	$(3)
  endef

  $$(eval $$(call BuildPackage,$(1)))
endef

$(eval $(call BuildPackage,libxtables))
$(eval $(call BuildPackage,libxtables-nft))
$(eval $(call BuildPackage,libip4tc))
$(eval $(call BuildPackage,libip6tc))
$(eval $(call BuildPackage,iptables))
$(eval $(call BuildPackage,iptables-nft))
$(eval $(call BuildPlugin,iptables-mod-conntrack-extra,$(IPT_CONNTRACK_EXTRA-m)))
$(eval $(call BuildPlugin,iptables-mod-conntrack-label,$(IPT_CONNTRACK_LABEL-m)))
$(eval $(call BuildPlugin,iptables-mod-extra,$(IPT_EXTRA-m)))
$(eval $(call BuildPlugin,iptables-mod-physdev,$(IPT_PHYSDEV-m)))
$(eval $(call BuildPlugin,iptables-mod-filter,$(IPT_FILTER-m)))
$(eval $(call BuildPlugin,iptables-mod-ipopt,$(IPT_IPOPT-m)))
$(eval $(call BuildPlugin,iptables-mod-ipsec,$(IPT_IPSEC-m)))
$(eval $(call BuildPlugin,iptables-mod-nat-extra,$(IPT_NAT_EXTRA-m)))
$(eval $(call BuildPlugin,iptables-mod-iprange,$(IPT_IPRANGE-m)))
$(eval $(call BuildPlugin,iptables-mod-cluster,$(IPT_CLUSTER-m)))
$(eval $(call BuildPlugin,iptables-mod-clusterip,$(IPT_CLUSTERIP-m)))
$(eval $(call BuildPlugin,iptables-mod-hashlimit,$(IPT_HASHLIMIT-m)))
$(eval $(call BuildPlugin,iptables-mod-rpfilter,$(IPT_RPFILTER-m)))
$(eval $(call BuildPlugin,iptables-mod-led,$(IPT_LED-m)))
$(eval $(call BuildPlugin,iptables-mod-socket,$(IPT_SOCKET-m)))
$(eval $(call BuildPlugin,iptables-mod-tproxy,$(IPT_TPROXY-m)))
$(eval $(call BuildPlugin,iptables-mod-tee,$(IPT_TEE-m)))
$(eval $(call BuildPlugin,iptables-mod-u32,$(IPT_U32-m)))
$(eval $(call BuildPlugin,iptables-mod-nflog,$(IPT_NFLOG-m)))
$(eval $(call BuildPlugin,iptables-mod-trace,$(IPT_DEBUG-m)))
$(eval $(call BuildPlugin,iptables-mod-nfqueue,$(IPT_NFQUEUE-m)))
$(eval $(call BuildPlugin,iptables-mod-checksum,$(IPT_CHECKSUM-m)))
$(eval $(call BuildPackage,ip6tables))
$(eval $(call BuildPackage,ip6tables-nft))
$(eval $(call BuildPlugin,ip6tables-extra,$(IPT_IPV6_EXTRA-m)))
$(eval $(call BuildPlugin,ip6tables-mod-nat,$(IPT_NAT6-m)))

