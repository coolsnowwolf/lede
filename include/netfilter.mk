# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2006-2020 OpenWrt.org

ifneq ($(__inc_netfilter),1)
__inc_netfilter:=1

ifeq ($(NF_KMOD),1)
P_V4:=ipv4/netfilter/
P_V6:=ipv6/netfilter/
P_XT:=netfilter/
P_EBT:=bridge/netfilter/
endif

# 1: variable
# 2: kconfig symbols
# 3: file list
# 4: version dependency
define nf_add
 $(if $(4),ifeq ($$(strip $$(call CompareKernelPatchVer,$$(KERNEL_PATCHVER),$(firstword $(4)),$(lastword $(4)))),1))
  $(1)-$$($(2)) += $(3)
 $(if $(4),endif)
 KCONFIG_$(1) = $(filter-out $(2),$(KCONFIG_$(1))) $(2)
endef


# core

# kernel only
$(eval $(if $(NF_KMOD),$(call nf_add,NF_REJECT,CONFIG_NF_REJECT_IPV4, $(P_V4)nf_reject_ipv4),))

$(eval $(if $(NF_KMOD),$(call nf_add,NF_IPT,CONFIG_IP_NF_IPTABLES, $(P_V4)ip_tables),))
$(eval $(if $(NF_KMOD),$(call nf_add,NF_IPT,CONFIG_NETFILTER_XTABLES, $(P_XT)x_tables),))

$(eval $(if $(NF_KMOD),$(call nf_add,IPT_CORE,CONFIG_NETFILTER_XTABLES, $(P_XT)xt_tcpudp),))
$(eval $(if $(NF_KMOD),$(call nf_add,IPT_CORE,CONFIG_IP_NF_FILTER, $(P_V4)iptable_filter),))
$(eval $(if $(NF_KMOD),$(call nf_add,IPT_CORE,CONFIG_IP_NF_MANGLE, $(P_V4)iptable_mangle),))

# userland only
$(eval $(if $(NF_KMOD),,$(call nf_add,IPT_CORE,CONFIG_IP_NF_IPTABLES, xt_standard ipt_icmp xt_tcp xt_udp xt_comment xt_set xt_SET)))

$(eval $(call nf_add,IPT_CORE,CONFIG_NETFILTER_XT_MATCH_LIMIT, $(P_XT)xt_limit))
$(eval $(call nf_add,IPT_CORE,CONFIG_NETFILTER_XT_MATCH_MAC, $(P_XT)xt_mac))
$(eval $(call nf_add,IPT_CORE,CONFIG_NETFILTER_XT_MATCH_MULTIPORT, $(P_XT)xt_multiport))
$(eval $(call nf_add,IPT_CORE,CONFIG_NETFILTER_XT_MATCH_COMMENT, $(P_XT)xt_comment))

#cluster
$(eval $(call nf_add,IPT_CLUSTER,CONFIG_NETFILTER_XT_MATCH_CLUSTER, $(P_XT)xt_cluster))

$(eval $(call nf_add,IPT_CORE,CONFIG_NETFILTER_XT_TARGET_LOG, $(P_XT)xt_LOG))
$(eval $(call nf_add,IPT_CORE,CONFIG_NETFILTER_XT_TARGET_TCPMSS, $(P_XT)xt_TCPMSS))
$(eval $(call nf_add,IPT_CORE,CONFIG_IP_NF_TARGET_REJECT, $(P_V4)ipt_REJECT))
$(eval $(call nf_add,IPT_CORE,CONFIG_NETFILTER_XT_MATCH_TIME, $(P_XT)xt_time))
$(eval $(call nf_add,IPT_CORE,CONFIG_NETFILTER_XT_MARK, $(P_XT)xt_mark))

# kernel has xt_MARK.ko merged into xt_mark.ko, userspace is still separate
# userland: xt_MARK.so
$(eval $(if $(NF_KMOD),,$(call nf_add,IPT_CORE,CONFIG_NETFILTER_XT_MARK, $(P_XT)xt_MARK)))


# conntrack

# kernel only
$(eval $(if $(NF_KMOD),$(call nf_add,NF_CONNTRACK,CONFIG_NF_CONNTRACK, $(P_XT)nf_conntrack),))
$(eval $(if $(NF_KMOD),$(call nf_add,NF_CONNTRACK,CONFIG_NF_DEFRAG_IPV4, $(P_V4)nf_defrag_ipv4),))

$(eval $(call nf_add,IPT_CONNTRACK,CONFIG_NETFILTER_XT_MATCH_STATE, $(P_XT)xt_state))
$(eval $(call nf_add,IPT_CONNTRACK,CONFIG_NETFILTER_XT_TARGET_CT, $(P_XT)xt_CT))
$(eval $(call nf_add,IPT_CONNTRACK,CONFIG_NETFILTER_XT_MATCH_CONNTRACK, $(P_XT)xt_conntrack))


# conntrack-extra

$(eval $(call nf_add,IPT_CONNTRACK_EXTRA,CONFIG_NETFILTER_XT_MATCH_CONNBYTES, $(P_XT)xt_connbytes))
$(eval $(call nf_add,IPT_CONNTRACK_EXTRA,CONFIG_NETFILTER_XT_MATCH_CONNLIMIT, $(P_XT)xt_connlimit))
$(eval $(call nf_add,IPT_CONNTRACK_EXTRA,CONFIG_NETFILTER_CONNCOUNT, $(P_XT)nf_conncount))
$(eval $(call nf_add,IPT_CONNTRACK_EXTRA,CONFIG_NETFILTER_XT_CONNMARK, $(P_XT)xt_connmark))
$(eval $(call nf_add,IPT_CONNTRACK_EXTRA,CONFIG_NETFILTER_XT_MATCH_HELPER, $(P_XT)xt_helper))
$(eval $(call nf_add,IPT_CONNTRACK_EXTRA,CONFIG_NETFILTER_XT_MATCH_RECENT, $(P_XT)xt_recent))

$(eval $(if $(NF_KMOD),,$(call nf_add,IPT_CONNTRACK_EXTRA,CONFIG_NETFILTER_XT_CONNMARK, $(P_XT)xt_CONNMARK)))

#conntrack-label

$(eval $(call nf_add,IPT_CONNTRACK_LABEL,CONFIG_NETFILTER_XT_MATCH_CONNLABEL, $(P_XT)xt_connlabel))

# extra

$(eval $(call nf_add,IPT_EXTRA,CONFIG_NETFILTER_XT_MATCH_ADDRTYPE, $(if $(NF_KMOD),$(P_XT)xt_addrtype,$(P_XT)ipt_addrtype)))
$(eval $(call nf_add,IPT_EXTRA,CONFIG_NETFILTER_XT_MATCH_OWNER, $(P_XT)xt_owner))
$(eval $(call nf_add,IPT_EXTRA,CONFIG_NETFILTER_XT_MATCH_PKTTYPE, $(P_XT)xt_pkttype))
$(eval $(call nf_add,IPT_EXTRA,CONFIG_NETFILTER_XT_MATCH_QUOTA, $(P_XT)xt_quota))
$(eval $(call nf_add,IPT_EXTRA,CONFIG_NETFILTER_XT_MATCH_CGROUP, $(P_XT)xt_cgroup))

#$(eval $(call nf_add,IPT_EXTRA,CONFIG_IP_NF_TARGET_ROUTE, $(P_V4)ipt_ROUTE))

# physdev

$(eval $(call nf_add,IPT_PHYSDEV,CONFIG_NETFILTER_XT_MATCH_PHYSDEV, $(P_XT)xt_physdev))

# filter

$(eval $(call nf_add,IPT_FILTER,CONFIG_NETFILTER_XT_MATCH_STRING, $(P_XT)xt_string))
$(eval $(call nf_add,IPT_FILTER,CONFIG_NETFILTER_XT_MATCH_BPF, $(P_XT)xt_bpf))


# ipopt

$(eval $(call nf_add,IPT_IPOPT,CONFIG_NETFILTER_XT_MATCH_DSCP, $(P_XT)xt_dscp))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_NETFILTER_XT_TARGET_DSCP, $(P_XT)xt_DSCP))
$(eval $(call nf_add,IPT_HASHLIMIT,CONFIG_NETFILTER_XT_MATCH_HASHLIMIT, $(P_XT)xt_hashlimit))
$(eval $(call nf_add,IPT_RPFILTER,CONFIG_IP_NF_MATCH_RPFILTER, $(P_V4)ipt_rpfilter))
$(eval $(call nf_add,IPT_RPFILTER,CONFIG_IP6_NF_MATCH_RPFILTER, $(P_V6)ip6t_rpfilter))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_NETFILTER_XT_MATCH_LENGTH, $(P_XT)xt_length))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_NETFILTER_XT_MATCH_STATISTIC, $(P_XT)xt_statistic))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_NETFILTER_XT_MATCH_TCPMSS, $(P_XT)xt_tcpmss))

$(eval $(call nf_add,IPT_IPOPT,CONFIG_NETFILTER_XT_TARGET_CLASSIFY, $(P_XT)xt_CLASSIFY))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_IP_NF_TARGET_ECN, $(P_V4)ipt_ECN))

$(eval $(call nf_add,IPT_IPOPT,CONFIG_NETFILTER_XT_MATCH_ECN, $(P_XT)xt_ecn))

# userland only
$(eval $(if $(NF_KMOD),,$(call nf_add,IPT_IPOPT,CONFIG_NETFILTER_XT_MATCH_DSCP, xt_tos)))
$(eval $(if $(NF_KMOD),,$(call nf_add,IPT_IPOPT,CONFIG_NETFILTER_XT_TARGET_DSCP, xt_TOS)))
$(eval $(if $(NF_KMOD),,$(call nf_add,IPT_IPOPT,CONFIG_NETFILTER_XT_MATCH_HL, ipt_ttl)))
$(eval $(if $(NF_KMOD),,$(call nf_add,IPT_IPOPT,CONFIG_NETFILTER_XT_TARGET_HL, ipt_TTL)))

$(eval $(call nf_add,IPT_IPOPT,CONFIG_NETFILTER_XT_MATCH_HL, $(P_XT)xt_hl))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_NETFILTER_XT_TARGET_HL, $(P_XT)xt_HL))

# iprange
$(eval $(call nf_add,IPT_IPRANGE,CONFIG_NETFILTER_XT_MATCH_IPRANGE, $(P_XT)xt_iprange))

#clusterip
$(eval $(call nf_add,IPT_CLUSTERIP,CONFIG_IP_NF_TARGET_CLUSTERIP, $(P_V4)ipt_CLUSTERIP))

# ipsec
$(eval $(call nf_add,IPT_IPSEC,CONFIG_IP_NF_MATCH_AH, $(P_V4)ipt_ah))
$(eval $(call nf_add,IPT_IPSEC,CONFIG_NETFILTER_XT_MATCH_ESP, $(P_XT)xt_esp))
$(eval $(call nf_add,IPT_IPSEC,CONFIG_NETFILTER_XT_MATCH_POLICY, $(P_XT)xt_policy))

# flow offload support
$(eval $(call nf_add,IPT_FLOW,CONFIG_NETFILTER_XT_TARGET_FLOWOFFLOAD, $(P_XT)xt_FLOWOFFLOAD))

# IPv6

# kernel only
$(eval $(if $(NF_KMOD),$(call nf_add,NF_REJECT6,CONFIG_NF_REJECT_IPV6, $(P_V6)nf_reject_ipv6),))

$(eval $(if $(NF_KMOD),$(call nf_add,NF_IPT6,CONFIG_IP6_NF_IPTABLES, $(P_V6)ip6_tables),))

$(eval $(if $(NF_KMOD),$(call nf_add,NF_CONNTRACK,CONFIG_NF_DEFRAG_IPV6, $(P_V6)nf_defrag_ipv6),))

$(eval $(if $(NF_KMOD),$(call nf_add,IPT_IPV6,CONFIG_IP6_NF_FILTER, $(P_V6)ip6table_filter),))
$(eval $(if $(NF_KMOD),$(call nf_add,IPT_IPV6,CONFIG_NF_LOG_IPV6, $(P_V6)nf_log_ipv6,lt 5.13),))

$(eval $(if $(NF_KMOD),,$(call nf_add,IPT_IPV6,CONFIG_IP6_NF_IPTABLES, ip6t_icmp6)))


$(eval $(call nf_add,IPT_IPV6,CONFIG_IP6_NF_TARGET_REJECT, $(P_V6)ip6t_REJECT))

# ipv6 extra
$(eval $(call nf_add,IPT_IPV6_EXTRA,CONFIG_IP6_NF_MATCH_IPV6HEADER, $(P_V6)ip6t_ipv6header))
$(eval $(call nf_add,IPT_IPV6_EXTRA,CONFIG_IP6_NF_MATCH_AH, $(P_V6)ip6t_ah))
$(eval $(call nf_add,IPT_IPV6_EXTRA,CONFIG_IP6_NF_MATCH_MH, $(P_V6)ip6t_mh))
$(eval $(call nf_add,IPT_IPV6_EXTRA,CONFIG_IP6_NF_MATCH_EUI64, $(P_V6)ip6t_eui64))
$(eval $(call nf_add,IPT_IPV6_EXTRA,CONFIG_IP6_NF_MATCH_OPTS, $(P_V6)ip6t_hbh))
$(eval $(call nf_add,IPT_IPV6_EXTRA,CONFIG_IP6_NF_MATCH_FRAG, $(P_V6)ip6t_frag))
$(eval $(call nf_add,IPT_IPV6_EXTRA,CONFIG_IP6_NF_MATCH_RT, $(P_V6)ip6t_rt))

# log

$(eval $(call nf_add,NF_LOG,CONFIG_NF_LOG_COMMON, $(P_XT)nf_log_common, lt 5.13))
$(eval $(call nf_add,NF_LOG,CONFIG_NF_LOG_IPV4, $(P_V4)nf_log_ipv4, lt 5.13))
$(eval $(call nf_add,NF_LOG,CONFIG_NF_LOG_SYSLOG, $(P_XT)nf_log_syslog, ge 5.13))
# $(eval $(if $(NF_KMOD),$(call nf_add,NF_LOG6,CONFIG_NF_LOG_IPV6, $(P_V6)nf_log_ipv6,lt 5.13),))

# nat

# kernel only
$(eval $(if $(NF_KMOD),$(call nf_add,NF_NAT,CONFIG_NF_NAT, $(P_XT)nf_nat),))

$(eval $(if $(NF_KMOD),$(call nf_add,NF_NAT,CONFIG_NF_NAT_IPV4, $(P_V4)nf_nat_ipv4, lt 5.1)))
$(eval $(if $(NF_KMOD),$(call nf_add,NF_NAT6,CONFIG_NF_NAT_IPV6, $(P_V6)nf_nat_ipv6, lt 5.1)))

$(eval $(if $(NF_KMOD),$(call nf_add,IPT_NAT,CONFIG_NETFILTER_XT_NAT, $(P_XT)xt_nat),))
$(eval $(if $(NF_KMOD),$(call nf_add,IPT_NAT,CONFIG_IP_NF_NAT, $(P_V4)iptable_nat),))
$(eval $(if $(NF_KMOD),$(call nf_add,IPT_NAT6,CONFIG_IP6_NF_NAT, $(P_V6)ip6table_nat),))
$(eval $(if $(NF_KMOD),$(call nf_add,IPT_NAT6,CONFIG_IP6_NF_TARGET_NPT, $(P_V6)ip6t_NPT),))

# userland only
$(eval $(if $(NF_KMOD),,$(call nf_add,IPT_NAT,CONFIG_NF_NAT, ipt_SNAT ipt_DNAT)))
$(eval $(if $(NF_KMOD),,$(call nf_add,IPT_NAT6,CONFIG_IP6_NF_TARGET_NPT, ip6t_DNPT ip6t_SNPT)))

$(eval $(call nf_add,IPT_NAT,CONFIG_NETFILTER_XT_TARGET_MASQUERADE, $(P_XT)xt_MASQUERADE))
$(eval $(call nf_add,IPT_NAT,CONFIG_NETFILTER_XT_TARGET_REDIRECT, $(P_XT)xt_REDIRECT))


# nat-extra

$(eval $(call nf_add,IPT_NAT_EXTRA,CONFIG_IP_NF_TARGET_NETMAP, $(P_XT)xt_NETMAP))


# nathelper

$(eval $(call nf_add,NF_NATHELPER,CONFIG_NF_CONNTRACK_FTP, $(P_XT)nf_conntrack_ftp))
$(eval $(call nf_add,NF_NATHELPER,CONFIG_NF_NAT_FTP, $(P_XT)nf_nat_ftp))


# nathelper-extra

$(eval $(call nf_add,NF_NATHELPER_EXTRA,CONFIG_NF_CONNTRACK_BROADCAST, $(P_XT)nf_conntrack_broadcast))
$(eval $(call nf_add,NF_NATHELPER_EXTRA,CONFIG_NF_CONNTRACK_AMANDA, $(P_XT)nf_conntrack_amanda))
$(eval $(call nf_add,NF_NATHELPER_EXTRA,CONFIG_NF_NAT_AMANDA, $(P_XT)nf_nat_amanda))
$(eval $(call nf_add,NF_NATHELPER_EXTRA,CONFIG_NF_CT_PROTO_GRE, $(P_XT)nf_conntrack_proto_gre, lt 5.1))
$(eval $(call nf_add,NF_NATHELPER_EXTRA,CONFIG_NF_NAT_PROTO_GRE, $(P_V4)nf_nat_proto_gre, lt 5.0))
$(eval $(call nf_add,NF_NATHELPER_EXTRA,CONFIG_NF_CONNTRACK_H323, $(P_XT)nf_conntrack_h323))
$(eval $(call nf_add,NF_NATHELPER_EXTRA,CONFIG_NF_NAT_H323, $(P_V4)nf_nat_h323))
$(eval $(call nf_add,NF_NATHELPER_EXTRA,CONFIG_NF_CONNTRACK_PPTP, $(P_XT)nf_conntrack_pptp))
$(eval $(call nf_add,NF_NATHELPER_EXTRA,CONFIG_NF_NAT_PPTP, $(P_V4)nf_nat_pptp))
$(eval $(call nf_add,NF_NATHELPER_EXTRA,CONFIG_NF_CONNTRACK_SIP, $(P_XT)nf_conntrack_sip))
$(eval $(call nf_add,NF_NATHELPER_EXTRA,CONFIG_NF_NAT_SIP, $(P_XT)nf_nat_sip))
$(eval $(call nf_add,NF_NATHELPER_EXTRA,CONFIG_NF_CONNTRACK_SNMP, $(P_XT)nf_conntrack_snmp))
$(eval $(call nf_add,NF_NATHELPER_EXTRA,CONFIG_NF_NAT_SNMP_BASIC, $(P_V4)nf_nat_snmp_basic))
$(eval $(call nf_add,NF_NATHELPER_EXTRA,CONFIG_NF_CONNTRACK_TFTP, $(P_XT)nf_conntrack_tftp))
$(eval $(call nf_add,NF_NATHELPER_EXTRA,CONFIG_NF_NAT_TFTP, $(P_XT)nf_nat_tftp))
$(eval $(call nf_add,NF_NATHELPER_EXTRA,CONFIG_NF_CONNTRACK_IRC, $(P_XT)nf_conntrack_irc))
$(eval $(call nf_add,NF_NATHELPER_EXTRA,CONFIG_NF_NAT_IRC, $(P_XT)nf_nat_irc))


# ulog

$(eval $(call nf_add,IPT_ULOG,CONFIG_IP_NF_TARGET_ULOG, $(P_V4)ipt_ULOG))


# nflog

$(eval $(call nf_add,IPT_NFLOG,CONFIG_NETFILTER_XT_TARGET_NFLOG, $(P_XT)xt_NFLOG))


# nfqueue

$(eval $(call nf_add,IPT_NFQUEUE,CONFIG_NETFILTER_XT_TARGET_NFQUEUE, $(P_XT)xt_NFQUEUE))


# debugging

$(eval $(call nf_add,IPT_DEBUG,CONFIG_NETFILTER_XT_TARGET_TRACE, $(P_XT)xt_TRACE))

# socket
$(eval $(call nf_add,NF_SOCKET,CONFIG_NF_SOCKET_IPV4, $(P_V4)nf_socket_ipv4))
$(eval $(call nf_add,NF_SOCKET,CONFIG_NF_SOCKET_IPV6, $(P_V6)nf_socket_ipv6))
$(eval $(call nf_add,IPT_SOCKET,CONFIG_NETFILTER_XT_MATCH_SOCKET, $(P_XT)xt_socket))

# tproxy
$(eval $(call nf_add,NF_TPROXY,CONFIG_NF_TPROXY_IPV4, $(P_V4)nf_tproxy_ipv4))
$(eval $(call nf_add,NF_TPROXY,CONFIG_NF_TPROXY_IPV6, $(P_V6)nf_tproxy_ipv6))
$(eval $(call nf_add,IPT_TPROXY,CONFIG_NETFILTER_XT_TARGET_TPROXY, $(P_XT)xt_TPROXY))

# led
$(eval $(call nf_add,IPT_LED,CONFIG_NETFILTER_XT_TARGET_LED, $(P_XT)xt_LED))

# tee

$(eval $(call nf_add,IPT_TEE,CONFIG_NETFILTER_XT_TARGET_TEE, $(P_XT)xt_TEE))
$(eval $(if $(NF_KMOD),$(call nf_add,IPT_TEE,CONFIG_NF_DUP_IPV4, $(P_V4)nf_dup_ipv4),))
$(eval $(if $(NF_KMOD),$(call nf_add,IPT_TEE,CONFIG_NF_DUP_IPV6, $(P_V6)nf_dup_ipv6),))

# u32

$(eval $(call nf_add,IPT_U32,CONFIG_NETFILTER_XT_MATCH_U32, $(P_XT)xt_u32))

# checksum

$(eval $(call nf_add,IPT_CHECKSUM,CONFIG_NETFILTER_XT_TARGET_CHECKSUM, $(P_XT)xt_CHECKSUM))


# netlink

$(eval $(call nf_add,NFNETLINK,CONFIG_NETFILTER_NETLINK, $(P_XT)nfnetlink))

# nflog

$(eval $(call nf_add,NFNETLINK_LOG,CONFIG_NETFILTER_NETLINK_LOG, $(P_XT)nfnetlink_log))

# nfqueue

$(eval $(call nf_add,NFNETLINK_QUEUE,CONFIG_NETFILTER_NETLINK_QUEUE, $(P_XT)nfnetlink_queue))

#
# ebtables
#

$(eval $(if $(NF_KMOD),$(call nf_add,EBTABLES,CONFIG_BRIDGE_NF_EBTABLES, $(P_EBT)ebtables),))

# ebtables: tables
$(eval $(call nf_add,EBTABLES,CONFIG_BRIDGE_EBT_BROUTE, $(P_EBT)ebtable_broute))
$(eval $(call nf_add,EBTABLES,CONFIG_BRIDGE_EBT_T_FILTER, $(P_EBT)ebtable_filter))
$(eval $(call nf_add,EBTABLES,CONFIG_BRIDGE_EBT_T_NAT, $(P_EBT)ebtable_nat))

# ebtables: matches
$(eval $(call nf_add,EBTABLES,CONFIG_BRIDGE_EBT_802_3, $(P_EBT)ebt_802_3))
$(eval $(call nf_add,EBTABLES,CONFIG_BRIDGE_EBT_AMONG, $(P_EBT)ebt_among))
$(eval $(call nf_add,EBTABLES_IP4,CONFIG_BRIDGE_EBT_ARP, $(P_EBT)ebt_arp))
$(eval $(call nf_add,EBTABLES_IP4,CONFIG_BRIDGE_EBT_IP, $(P_EBT)ebt_ip))
$(eval $(call nf_add,EBTABLES_IP6,CONFIG_BRIDGE_EBT_IP6, $(P_EBT)ebt_ip6))
$(eval $(call nf_add,EBTABLES,CONFIG_BRIDGE_EBT_LIMIT, $(P_EBT)ebt_limit))
$(eval $(call nf_add,EBTABLES,CONFIG_BRIDGE_EBT_MARK, $(P_EBT)ebt_mark_m))
$(eval $(call nf_add,EBTABLES,CONFIG_BRIDGE_EBT_PKTTYPE, $(P_EBT)ebt_pkttype))
$(eval $(call nf_add,EBTABLES,CONFIG_BRIDGE_EBT_STP, $(P_EBT)ebt_stp))
$(eval $(call nf_add,EBTABLES,CONFIG_BRIDGE_EBT_VLAN, $(P_EBT)ebt_vlan))

# targets
$(eval $(call nf_add,EBTABLES_IP4,CONFIG_BRIDGE_EBT_ARPREPLY, $(P_EBT)ebt_arpreply))
$(eval $(call nf_add,EBTABLES,CONFIG_BRIDGE_EBT_MARK_T, $(P_EBT)ebt_mark))
$(eval $(call nf_add,EBTABLES_IP4,CONFIG_BRIDGE_EBT_DNAT, $(P_EBT)ebt_dnat))
$(eval $(call nf_add,EBTABLES,CONFIG_BRIDGE_EBT_REDIRECT, $(P_EBT)ebt_redirect))
$(eval $(call nf_add,EBTABLES_IP4,CONFIG_BRIDGE_EBT_SNAT, $(P_EBT)ebt_snat))

# watchers
$(eval $(call nf_add,EBTABLES_WATCHERS,CONFIG_BRIDGE_EBT_LOG, $(P_EBT)ebt_log))
$(eval $(call nf_add,EBTABLES_WATCHERS,CONFIG_BRIDGE_EBT_ULOG, $(P_EBT)ebt_ulog))
$(eval $(call nf_add,EBTABLES_WATCHERS,CONFIG_BRIDGE_EBT_NFLOG, $(P_EBT)ebt_nflog))
$(eval $(call nf_add,EBTABLES_WATCHERS,CONFIG_BRIDGE_EBT_NFQUEUE, $(P_EBT)ebt_nfqueue))

# nftables
$(eval $(if $(NF_KMOD),$(call nf_add,NFT_CORE,CONFIG_NF_TABLES, $(P_XT)nf_tables),))
$(eval $(if $(NF_KMOD),$(call nf_add,NFT_CORE,CONFIG_NF_TABLES_SET, $(P_XT)nf_tables_set),))
$(eval $(if $(NF_KMOD),$(call nf_add,NFT_CORE,CONFIG_NFT_COUNTER, $(P_XT)nft_counter),))
$(eval $(if $(NF_KMOD),$(call nf_add,NFT_CORE,CONFIG_NFT_CT, $(P_XT)nft_ct),))
$(eval $(if $(NF_KMOD),$(call nf_add,NFT_CORE,CONFIG_NFT_HASH, $(P_XT)nft_hash),))
$(eval $(if $(NF_KMOD),$(call nf_add,NFT_CORE,CONFIG_NFT_LIMIT, $(P_XT)nft_limit),))
$(eval $(if $(NF_KMOD),$(call nf_add,NFT_CORE,CONFIG_NFT_LOG, $(P_XT)nft_log),))
$(eval $(if $(NF_KMOD),$(call nf_add,NFT_CORE,CONFIG_NFT_META, $(P_XT)nft_meta),))
$(eval $(if $(NF_KMOD),$(call nf_add,NFT_CORE,CONFIG_NFT_NUMGEN, $(P_XT)nft_numgen),))
$(eval $(if $(NF_KMOD),$(call nf_add,NFT_CORE,CONFIG_NFT_OBJREF, $(P_XT)nft_objref),))
$(eval $(if $(NF_KMOD),$(call nf_add,NFT_CORE,CONFIG_NFT_QUOTA, $(P_XT)nft_quota),))
$(eval $(if $(NF_KMOD),$(call nf_add,NFT_CORE,CONFIG_NFT_REDIR, $(P_XT)nft_redir),))
$(eval $(if $(NF_KMOD),$(call nf_add,NFT_CORE,CONFIG_NFT_REJECT, $(P_XT)nft_reject $(P_V4)nft_reject_ipv4 $(P_V6)nft_reject_ipv6),))
$(eval $(if $(NF_KMOD),$(call nf_add,NFT_CORE,CONFIG_NFT_REJECT_INET, $(P_XT)nft_reject_inet),))

$(eval $(if $(NF_KMOD),$(call nf_add,NFT_BRIDGE,CONFIG_NFT_BRIDGE_META, $(P_EBT)nft_meta_bridge),))
$(eval $(if $(NF_KMOD),$(call nf_add,NFT_BRIDGE,CONFIG_NFT_BRIDGE_REJECT, $(P_EBT)nft_reject_bridge),))
$(eval $(if $(NF_KMOD),$(call nf_add,NFT_BRIDGE,CONFIG_NF_CONNTRACK_BRIDGE, $(P_EBT)nf_conntrack_bridge),))

$(eval $(if $(NF_KMOD),$(call nf_add,NFT_NAT,CONFIG_NFT_NAT, $(P_XT)nft_nat),))
$(eval $(if $(NF_KMOD),$(call nf_add,NFT_NAT,CONFIG_NFT_NAT, $(P_XT)nft_chain_nat),))
$(eval $(if $(NF_KMOD),$(call nf_add,NFT_NAT,CONFIG_NFT_REDIR_IPV4, $(P_V4)nft_redir_ipv4),))
$(eval $(if $(NF_KMOD),$(call nf_add,NFT_NAT,CONFIG_NFT_MASQ, $(P_XT)nft_masq),))
$(eval $(if $(NF_KMOD),$(call nf_add,NFT_NAT,CONFIG_NFT_MASQ_IPV4, $(P_V4)nft_masq_ipv4),))

$(eval $(if $(NF_KMOD),$(call nf_add,NFT_NAT6,CONFIG_NFT_REDIR_IPV6, $(P_V6)nft_redir_ipv6),))
$(eval $(if $(NF_KMOD),$(call nf_add,NFT_NAT6,CONFIG_NFT_MASQ_IPV6, $(P_V6)nft_masq_ipv6),))

$(eval $(if $(NF_KMOD),$(call nf_add,NFT_FIB,CONFIG_NFT_FIB, $(P_XT)nft_fib),))
$(eval $(if $(NF_KMOD),$(call nf_add,NFT_FIB,CONFIG_NFT_FIB_INET, $(P_XT)nft_fib_inet),))
$(eval $(if $(NF_KMOD),$(call nf_add,NFT_FIB,CONFIG_NFT_FIB_IPV4, $(P_V4)nft_fib_ipv4),))
$(eval $(if $(NF_KMOD),$(call nf_add,NFT_FIB,CONFIG_NFT_FIB_IPV6, $(P_V6)nft_fib_ipv6),))

$(eval $(if $(NF_KMOD),$(call nf_add,NFT_QUEUE,CONFIG_NFT_QUEUE, $(P_XT)nft_queue),))

$(eval $(if $(NF_KMOD),$(call nf_add,NFT_SOCKET,CONFIG_NFT_SOCKET, $(P_XT)nft_socket),))

$(eval $(if $(NF_KMOD),$(call nf_add,NFT_TPROXY,CONFIG_NFT_TPROXY, $(P_XT)nft_tproxy),))

$(eval $(if $(NF_KMOD),$(call nf_add,NFT_COMPAT,CONFIG_NFT_COMPAT, $(P_XT)nft_compat),))

$(eval $(if $(NF_KMOD),$(call nf_add,NFT_XFRM,CONFIG_NFT_XFRM, $(P_XT)nft_xfrm),))

# userland only
IPT_BUILTIN += $(NF_IPT-y) $(NF_IPT-m)
IPT_BUILTIN += $(IPT_CORE-y) $(IPT_CORE-m)
IPT_BUILTIN += $(NF_CONNTRACK-y)
IPT_BUILTIN += $(NF_CONNTRACK6-y)
IPT_BUILTIN += $(IPT_CONNTRACK-y)
IPT_BUILTIN += $(IPT_CONNTRACK_EXTRA-y)
IPT_BUILTIN += $(IPT_EXTRA-y)
IPT_BUILTIN += $(IPT_PHYSDEV-y)
IPT_BUILTIN += $(IPT_FILTER-y)
IPT_BUILTIN += $(IPT_FLOW-y) $(IPT_FLOW-m)
IPT_BUILTIN += $(IPT_IPOPT-y)
IPT_BUILTIN += $(IPT_IPRANGE-y)
IPT_BUILTIN += $(IPT_CLUSTER-y)
IPT_BUILTIN += $(IPT_CLUSTERIP-y)
IPT_BUILTIN += $(IPT_IPSEC-y)
IPT_BUILTIN += $(IPT_IPV6-y) $(IPT_IPV6-m)
IPT_BUILTIN += $(NF_NAT-y)
IPT_BUILTIN += $(NF_NAT6-y)
IPT_BUILTIN += $(IPT_NAT-y)
IPT_BUILTIN += $(IPT_NAT6-y)
IPT_BUILTIN += $(IPT_NAT_EXTRA-y)
IPT_BUILTIN += $(NF_NATHELPER-y)
IPT_BUILTIN += $(NF_NATHELPER_EXTRA-y)
IPT_BUILTIN += $(IPT_ULOG-y)
IPT_BUILTIN += $(IPT_TPROXY-y)
IPT_BUILTIN += $(NFNETLINK-y)
IPT_BUILTIN += $(NFNETLINK_LOG-y)
IPT_BUILTIN += $(NFNETLINK_QUEUE-y)
IPT_BUILTIN += $(EBTABLES-y)
IPT_BUILTIN += $(EBTABLES_IP4-y)
IPT_BUILTIN += $(EBTABLES_IP6-y)
IPT_BUILTIN += $(EBTABLES_WATCHERS-y)

endif # __inc_netfilter
