#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=iptables
PKG_VERSION:=1.8.10
PKG_RELEASE:=3

PKG_SOURCE_URL:=https://netfilter.org/projects/iptables/files
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_HASH:=5cc255c189356e317d070755ce9371eb63a1b783c34498fb8c30264f3cc59c9c

PKG_FIXUP:=autoreconf
PKG_FLAGS:=nonshared

PKG_INSTALL:=1
PKG_BUILD_FLAGS:=gc-sections no-lto
PKG_BUILD_PARALLEL:=1
PKG_LICENSE:=GPL-2.0
PKG_CPE_ID:=cpe:/a:netfilter:iptables

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
  DEPENDS:=+libxtables $(1)
endef

define Package/xtables-legacy
$(call Package/iptables/Default)
  TITLE:=IP firewall administration tool
  DEPENDS+= +kmod-ipt-core +libip4tc +IPV6:libip6tc +libiptext +IPV6:libiptext6 +libxtables
endef

define Package/iptables-zz-legacy
$(call Package/iptables/Default)
  TITLE:=IP firewall administration tool
  DEPENDS+= +xtables-legacy
  PROVIDES:=iptables iptables-legacy
  DEFAULT_VARIANT:=1
  ALTERNATIVES:=\
    300:/usr/sbin/iptables:xtables-legacy-multi \
    300:/usr/sbin/iptables-restore:xtables-legacy-multi \
    300:/usr/sbin/iptables-save:xtables-legacy-multi \
    300:/usr/sbin/iptables-legacy:xtables-legacy-multi \
    300:/usr/sbin/iptables-legacy-restore:xtables-legacy-multi \
    300:/usr/sbin/iptables-legacy-save:xtables-legacy-multi
endef

define Package/iptables-zz-legacy/description
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

define Package/xtables-nft
$(call Package/iptables/Default)
  TITLE:=IP firewall administration tool nft
  DEPENDS:=+libnftnl +libiptext +IPV6:libiptext6 +libiptext-nft +kmod-nft-compat
endef

define Package/arptables-nft
$(call Package/iptables/Default)
  DEPENDS:=+kmod-nft-arp +xtables-nft +kmod-arptables
  TITLE:=ARP firewall administration tool nft
  PROVIDES:=arptables
  ALTERNATIVES:=\
    300:/usr/sbin/arptables:xtables-nft-multi \
    300:/usr/sbin/arptables-restore:xtables-nft-multi \
    300:/usr/sbin/arptables-save:xtables-nft-multi \
    300:/usr/sbin/arptables-nft:xtables-nft-multi \
    300:/usr/sbin/arptables-nft-restore:xtables-nft-multi \
    300:/usr/sbin/arptables-nft-save:xtables-nft-multi
endef

define Package/ebtables-nft
$(call Package/iptables/Default)
  DEPENDS:=+kmod-nft-bridge +xtables-nft +kmod-ebtables
  TITLE:=Bridge firewall administration tool nft
  PROVIDES:=ebtables
  ALTERNATIVES:=\
    300:/usr/sbin/ebtables:xtables-nft-multi \
    300:/usr/sbin/ebtables-restore:xtables-nft-multi \
    300:/usr/sbin/ebtables-save:xtables-nft-multi \
    300:/usr/sbin/ebtables-nft:xtables-nft-multi \
    300:/usr/sbin/ebtables-nft-restore:xtables-nft-multi \
    300:/usr/sbin/ebtables-nft-save:xtables-nft-multi
endef

define Package/iptables-nft
$(call Package/iptables/Default)
  TITLE:=IP firewall administration tool nft
  DEPENDS:=+kmod-ipt-core +xtables-nft
  PROVIDES:=iptables
  ALTERNATIVES:=\
    200:/usr/sbin/iptables:xtables-nft-multi \
    200:/usr/sbin/iptables-restore:xtables-nft-multi \
    200:/usr/sbin/iptables-save:xtables-nft-multi \
    200:/usr/sbin/iptables-nft:xtables-nft-multi \
    200:/usr/sbin/iptables-nft-restore:xtables-nft-multi \
    200:/usr/sbin/iptables-nft-save:xtables-nft-multi \
    200:/usr/sbin/iptables-restore-translate:xtables-nft-multi \
    200:/usr/sbin/iptables-translate:xtables-nft-multi
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
$(call Package/iptables/Module, +kmod-ipt-conntrack-extra)
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

define Package/ip6tables-zz-legacy
$(call Package/iptables/Default)
  DEPENDS:=@IPV6 +kmod-ip6tables +xtables-legacy
  CATEGORY:=Network
  TITLE:=IPv6 firewall administration tool
  PROVIDES:=ip6tables ip6tables-legacy
  DEFAULT_VARIANT:=1
  ALTERNATIVES:=\
    300:/usr/sbin/ip6tables:xtables-legacy-multi \
    300:/usr/sbin/ip6tables-restore:xtables-legacy-multi \
    300:/usr/sbin/ip6tables-save:xtables-legacy-multi \
    300:/usr/sbin/ip6tables-legacy:xtables-legacy-multi \
    300:/usr/sbin/ip6tables-legacy-restore:xtables-legacy-multi \
    300:/usr/sbin/ip6tables-legacy-save:xtables-legacy-multi
endef

define Package/ip6tables-nft
$(call Package/iptables/Default)
  DEPENDS:=@IPV6 +kmod-ip6tables +xtables-nft
  TITLE:=IP firewall administration tool nft
  PROVIDES:=ip6tables
  ALTERNATIVES:=\
    200:/usr/sbin/ip6tables:xtables-nft-multi \
    200:/usr/sbin/ip6tables-restore:xtables-nft-multi \
    200:/usr/sbin/ip6tables-save:xtables-nft-multi \
    200:/usr/sbin/ip6tables-nft:xtables-nft-multi \
    200:/usr/sbin/ip6tables-nft-restore:xtables-nft-multi \
    200:/usr/sbin/ip6tables-nft-save:xtables-nft-multi \
    200:/usr/sbin/ip6tables-restore-translate:xtables-nft-multi \
    200:/usr/sbin/ip6tables-translate:xtables-nft-multi
endef

define Package/ip6tables-nft/description
Extra ip6tables nftables nft binaries.
  ip6tables-nft
  ip6tables-nft-restore
  ip6tables-nft-save
  ip6tables-translate
  ip6tables-restore-translate
endef

define Package/ip6tables-extra
$(call Package/iptables/Default)
  DEPENDS:=+libxtables +kmod-ip6tables-extra
  TITLE:=IPv6 header matching modules
endef

define Package/ip6tables-extra/description
iptables header matching modules for IPv6
endef

define Package/ip6tables-mod-nat
$(call Package/iptables/Default)
  DEPENDS:=+libxtables +kmod-ipt-nat6
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
endef

define Package/libip6tc
$(call Package/iptables/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=IPv6 firewall - shared libiptc library
  ABI_VERSION:=2
endef

define Package/libiptext
 $(call Package/iptables/Default)
 SECTION:=libs
 CATEGORY:=Libraries
 TITLE:=IPv4 firewall - shared libiptext library
 ABI_VERSION:=0
 DEPENDS:=+libxtables
endef

define Package/libiptext6
 $(call Package/iptables/Default)
 SECTION:=libs
 CATEGORY:=Libraries
 TITLE:=IPv6 firewall - shared libiptext library
 ABI_VERSION:=0
 DEPENDS:=+libxtables
endef

define Package/libiptext-nft
 $(call Package/iptables/Default)
 SECTION:=libs
 CATEGORY:=Libraries
 TITLE:=IPv4/IPv6 firewall - shared libiptext nft library
 ABI_VERSION:=0
 DEPENDS:=+libxtables
endef

define Package/libxtables
 $(call Package/iptables/Default)
 SECTION:=libs
 CATEGORY:=Libraries
 TITLE:=IPv4/IPv6 firewall - shared xtables library
 MENU:=1
 ABI_VERSION:=12
 DEPENDS:=+IPTABLES_CONNLABEL:libnetfilter-conntrack
endef

define Package/libxtables/config
  config IPTABLES_CONNLABEL
	bool "Enable Connlabel support"
	default n
	help
		This enable connlabel support in iptables.
endef

TARGET_CPPFLAGS := \
	-I$(PKG_BUILD_DIR)/include \
	-I$(LINUX_DIR)/user_headers/include \
	$(TARGET_CPPFLAGS)

TARGET_CFLAGS += \
	-I$(PKG_BUILD_DIR)/include \
	-I$(LINUX_DIR)/user_headers/include \
	-DNO_LEGACY

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \
	--enable-devel \
	--with-kernel="$(LINUX_DIR)/user_headers" \
	--with-xtlibdir=/usr/lib/iptables \
	--with-xt-lock-name=/var/run/xtables.lock \
	$(if $(CONFIG_IPTABLES_CONNLABEL),,--disable-connlabel) \
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

define Package/xtables-legacy/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/xtables-legacy-multi $(1)/usr/sbin/
endef

define Package/xtables-nft/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/xtables-nft-multi $(1)/usr/sbin/
endef

define Package/arptables-nft/install
	$(INSTALL_DIR) $(1)/usr/lib/iptables
	$(CP) $(PKG_BUILD_DIR)/extensions/libarpt_*.so $(1)/usr/lib/iptables/
endef

define Package/ebtables-nft/install
	$(INSTALL_DIR) $(1)/usr/lib/iptables
	$(CP) $(PKG_BUILD_DIR)/extensions/libebt_*.so $(1)/usr/lib/iptables/
endef

define Package/iptables-zz-legacy/install
	true
endef

define Package/iptables-nft/install
	true
endef

define Package/ip6tables-zz-legacy/install
	true
endef

define Package/ip6tables-nft/install
	true
endef

define Package/libip4tc/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libip4tc.so.* $(1)/usr/lib/
endef

define Package/libip6tc/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libip6tc.so.* $(1)/usr/lib/
endef

define Package/libiptext/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/extensions/libiptext.so $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/extensions/libiptext4.so $(1)/usr/lib/
endef

define Package/libiptext6/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/extensions/libiptext6.so $(1)/usr/lib/
endef

define Package/libiptext-nft/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/extensions/libiptext_*.so $(1)/usr/lib/
endef

define Package/libxtables/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libxtables.so.* $(1)/usr/lib/
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
$(eval $(call BuildPackage,libip4tc))
$(eval $(call BuildPackage,libip6tc))
$(eval $(call BuildPackage,libiptext))
$(eval $(call BuildPackage,libiptext6))
$(eval $(call BuildPackage,libiptext-nft))
$(eval $(call BuildPackage,xtables-legacy))
$(eval $(call BuildPackage,xtables-nft))
$(eval $(call BuildPackage,arptables-nft))
$(eval $(call BuildPackage,ebtables-nft))
$(eval $(call BuildPackage,iptables-nft))
$(eval $(call BuildPackage,iptables-zz-legacy))
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
$(eval $(call BuildPackage,ip6tables-nft))
$(eval $(call BuildPackage,ip6tables-zz-legacy))
$(eval $(call BuildPlugin,ip6tables-extra,$(IPT_IPV6_EXTRA-m)))
$(eval $(call BuildPlugin,ip6tables-mod-nat,$(IPT_NAT6-m)))
