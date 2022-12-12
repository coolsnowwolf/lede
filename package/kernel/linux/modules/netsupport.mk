#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

NETWORK_SUPPORT_MENU:=Network Support

define KernelPackage/atm
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=ATM support
  KCONFIG:= \
	CONFIG_ATM \
	CONFIG_ATM_BR2684
  FILES:= \
	$(LINUX_DIR)/net/atm/atm.ko \
	$(LINUX_DIR)/net/atm/br2684.ko
  AUTOLOAD:=$(call AutoLoad,30,atm br2684)
endef

define KernelPackage/atm/description
 Kernel modules for ATM support
endef

$(eval $(call KernelPackage,atm))


define KernelPackage/atmtcp
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=ATM over TCP
  DEPENDS:=kmod-atm
  KCONFIG:=CONFIG_ATM_TCP CONFIG_ATM_DRIVERS=y
  FILES:=$(LINUX_DIR)/drivers/atm/atmtcp.ko
  AUTOLOAD:=$(call AutoLoad,40,atmtcp)
endef

define KernelPackage/atmtcp/description
 Kernel module for ATM over TCP support
endef

$(eval $(call KernelPackage,atmtcp))


define KernelPackage/bonding
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=Ethernet bonding driver
  KCONFIG:=CONFIG_BONDING
  FILES:=$(LINUX_DIR)/drivers/net/bonding/bonding.ko
  AUTOLOAD:=$(call AutoLoad,40,bonding)
  MODPARAMS.bonding:=max_bonds=0
endef

define KernelPackage/bonding/description
 Kernel module for NIC bonding.
endef

$(eval $(call KernelPackage,bonding))


define KernelPackage/udptunnel4
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=IPv4 UDP tunneling support
  KCONFIG:=CONFIG_NET_UDP_TUNNEL
  HIDDEN:=1
  FILES:=$(LINUX_DIR)/net/ipv4/udp_tunnel.ko
  AUTOLOAD:=$(call AutoLoad,32,udp_tunnel)
endef


$(eval $(call KernelPackage,udptunnel4))

define KernelPackage/udptunnel6
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=IPv6 UDP tunneling support
  DEPENDS:=@IPV6
  KCONFIG:=CONFIG_NET_UDP_TUNNEL
  HIDDEN:=1
  FILES:=$(LINUX_DIR)/net/ipv6/ip6_udp_tunnel.ko
  AUTOLOAD:=$(call AutoLoad,32,ip6_udp_tunnel)
endef

$(eval $(call KernelPackage,udptunnel6))


define KernelPackage/vxlan
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=Native VXLAN Kernel support
  DEPENDS:= \
	+kmod-iptunnel \
	+kmod-udptunnel4 \
	+IPV6:kmod-udptunnel6
  KCONFIG:=CONFIG_VXLAN
  FILES:=$(LINUX_DIR)/drivers/net/vxlan.ko@lt5.18 \
          $(LINUX_DIR)/drivers/net/vxlan/vxlan.ko@ge5.18
  AUTOLOAD:=$(call AutoLoad,13,vxlan)
endef

define KernelPackage/vxlan/description
 Kernel module for supporting VXLAN in the Kernel.
 Requires Kernel 3.12 or newer.
endef

$(eval $(call KernelPackage,vxlan))


define KernelPackage/geneve
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=Generic Network Virtualization Encapsulation (Geneve) support
  DEPENDS:= \
	+kmod-iptunnel \
	+kmod-udptunnel4 \
	+IPV6:kmod-udptunnel6
  KCONFIG:=CONFIG_GENEVE
  FILES:= \
	$(LINUX_DIR)/drivers/net/geneve.ko
  AUTOLOAD:=$(call AutoLoad,13,geneve)
endef

define KernelPackage/geneve/description
 Kernel module for supporting Geneve in the Kernel.
 Requires Kernel 3.18 or newer.
endef

$(eval $(call KernelPackage,geneve))


define KernelPackage/nsh
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=Network Service Header (NSH) protocol
  DEPENDS:=
  KCONFIG:=CONFIG_NET_NSH
  FILES:=$(LINUX_DIR)/net/nsh/nsh.ko
  AUTOLOAD:=$(call AutoLoad,13,nsh)
endef

define KernelPackage/nsh/description
  Network Service Header is an implementation of Service Function
  Chaining (RFC 7665).  Requires kernel 4.14 or newer
endef

$(eval $(call KernelPackage,nsh))


define KernelPackage/misdn
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=mISDN (ISDN) Support
  KCONFIG:= \
	CONFIG_ISDN=y \
  	CONFIG_MISDN \
	CONFIG_MISDN_DSP \
	CONFIG_MISDN_L1OIP
  FILES:= \
  	$(LINUX_DIR)/drivers/isdn/mISDN/mISDN_core.ko \
	$(LINUX_DIR)/drivers/isdn/mISDN/mISDN_dsp.ko \
	$(LINUX_DIR)/drivers/isdn/mISDN/l1oip.ko
  AUTOLOAD:=$(call AutoLoad,30,mISDN_core mISDN_dsp l1oip)
endef

define KernelPackage/misdn/description
  Modular ISDN driver support
endef

$(eval $(call KernelPackage,misdn))


define KernelPackage/isdn4linux
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=Old ISDN4Linux (deprecated)
  DEPENDS:=+kmod-ppp
  KCONFIG:= \
	CONFIG_ISDN=y \
    CONFIG_ISDN_I4L \
    CONFIG_ISDN_PPP=y \
    CONFIG_ISDN_PPP_VJ=y \
    CONFIG_ISDN_MPP=y \
    CONFIG_IPPP_FILTER=y \
    CONFIG_ISDN_PPP_BSDCOMP \
    CONFIG_ISDN_CAPI_MIDDLEWARE=y \
    CONFIG_ISDN_CAPI_CAPIFS_BOOL=y \
    CONFIG_ISDN_AUDIO=y \
    CONFIG_ISDN_TTY_FAX=y \
    CONFIG_ISDN_X25=y \
    CONFIG_ISDN_DIVERSION
  FILES:= \
    $(LINUX_DIR)/drivers/isdn/divert/dss1_divert.ko \
	$(LINUX_DIR)/drivers/isdn/i4l/isdn.ko \
	$(LINUX_DIR)/drivers/isdn/i4l/isdn_bsdcomp.ko
  AUTOLOAD:=$(call AutoLoad,40,isdn isdn_bsdcomp dss1_divert)
endef

define KernelPackage/isdn4linux/description
  This driver allows you to use an ISDN adapter for networking
endef

$(eval $(call KernelPackage,isdn4linux))


define KernelPackage/ipip
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=IP-in-IP encapsulation
  DEPENDS:=+kmod-iptunnel +kmod-iptunnel4
  KCONFIG:=CONFIG_NET_IPIP
  FILES:=$(LINUX_DIR)/net/ipv4/ipip.ko
  AUTOLOAD:=$(call AutoLoad,32,ipip)
endef

define KernelPackage/ipip/description
 Kernel modules for IP-in-IP encapsulation
endef

$(eval $(call KernelPackage,ipip))


IPSEC-m:= \
	xfrm/xfrm_algo \
	xfrm/xfrm_ipcomp \
	xfrm/xfrm_user \
	key/af_key \

define KernelPackage/ipsec
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=IPsec related modules (IPv4 and IPv6)
  DEPENDS:= \
	+kmod-crypto-authenc +kmod-crypto-cbc +kmod-crypto-deflate \
	+kmod-crypto-des +kmod-crypto-echainiv +kmod-crypto-hmac \
	+kmod-crypto-md5 +kmod-crypto-sha1
  KCONFIG:= \
	CONFIG_NET_KEY \
	CONFIG_XFRM_USER \
	CONFIG_INET_IPCOMP \
	CONFIG_XFRM_IPCOMP
  FILES:=$(foreach mod,$(IPSEC-m),$(LINUX_DIR)/net/$(mod).ko)
  AUTOLOAD:=$(call AutoLoad,30,$(notdir $(IPSEC-m)))
endef

define KernelPackage/ipsec/description
 Kernel modules for IPsec support in both IPv4 and IPv6.
 Includes:
 - af_key
 - xfrm_algo
 - xfrm_ipcomp
 - xfrm_user
endef

$(eval $(call KernelPackage,ipsec))

IPSEC4-m = \
	ipv4/ah4 \
	ipv4/esp4 \
	ipv4/xfrm4_tunnel \
	ipv4/ipcomp \

IPSEC4-m += $(ifeq ($$(strip $$(call CompareKernelPatchVer,$$(KERNEL_PATCHVER),le,5.2))),ipv4/xfrm4_mode_beet ipv4/xfrm4_mode_transport ipv4/xfrm4_mode_tunnel)

define KernelPackage/ipsec4
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=IPsec related modules (IPv4)
  DEPENDS:=kmod-ipsec +kmod-iptunnel4
  KCONFIG:= \
	CONFIG_INET_AH \
	CONFIG_INET_ESP \
	CONFIG_INET_IPCOMP \
	CONFIG_INET_XFRM_MODE_BEET \
	CONFIG_INET_XFRM_MODE_TRANSPORT \
	CONFIG_INET_XFRM_MODE_TUNNEL \
	CONFIG_INET_XFRM_TUNNEL \
	CONFIG_INET_ESP_OFFLOAD=n
  FILES:=$(foreach mod,$(IPSEC4-m),$(LINUX_DIR)/net/$(mod).ko)
  AUTOLOAD:=$(call AutoLoad,32,$(notdir $(IPSEC4-m)))
endef

define KernelPackage/ipsec4/description
 Kernel modules for IPsec support in IPv4.
 Includes:
 - ah4
 - esp4
 - ipcomp4
 - xfrm4_mode_beet
 - xfrm4_mode_transport
 - xfrm4_mode_tunnel
 - xfrm4_tunnel
endef

$(eval $(call KernelPackage,ipsec4))


IPSEC6-m = \
	ipv6/ah6 \
	ipv6/esp6 \
	ipv6/xfrm6_tunnel \
	ipv6/ipcomp6 \

IPSEC6-m += $(ifeq ($$(strip $$(call CompareKernelPatchVer,$$(KERNEL_PATCHVER),le,5.2))),ipv6/xfrm6_mode_beet ipv6/xfrm6_mode_transport ipv6/xfrm6_mode_tunnel)

define KernelPackage/ipsec6
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=IPsec related modules (IPv6)
  DEPENDS:=@IPV6 kmod-ipsec +kmod-iptunnel6
  KCONFIG:= \
	CONFIG_INET6_AH \
	CONFIG_INET6_ESP \
	CONFIG_INET6_IPCOMP \
	CONFIG_INET6_XFRM_MODE_BEET \
	CONFIG_INET6_XFRM_MODE_TRANSPORT \
	CONFIG_INET6_XFRM_MODE_TUNNEL \
	CONFIG_INET6_XFRM_TUNNEL \
	CONFIG_INET6_ESP_OFFLOAD=n
  FILES:=$(foreach mod,$(IPSEC6-m),$(LINUX_DIR)/net/$(mod).ko)
  AUTOLOAD:=$(call AutoLoad,32,$(notdir $(IPSEC6-m)))
endef

define KernelPackage/ipsec6/description
 Kernel modules for IPsec support in IPv6.
 Includes:
 - ah6
 - esp6
 - ipcomp6
 - xfrm6_mode_beet
 - xfrm6_mode_transport
 - xfrm6_mode_tunnel
 - xfrm6_tunnel
endef

$(eval $(call KernelPackage,ipsec6))


define KernelPackage/iptunnel
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=IP tunnel support
  HIDDEN:=1
  KCONFIG:= \
	CONFIG_NET_IP_TUNNEL
  FILES:=$(LINUX_DIR)/net/ipv4/ip_tunnel.ko
  AUTOLOAD:=$(call AutoLoad,31,ip_tunnel)
endef

define KernelPackage/iptunnel/description
 Kernel module for generic IP tunnel support
endef

$(eval $(call KernelPackage,iptunnel))


define KernelPackage/ip-vti
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=IP VTI (Virtual Tunnel Interface)
  DEPENDS:=+kmod-iptunnel +kmod-iptunnel4 +kmod-ipsec4
  KCONFIG:=CONFIG_NET_IPVTI
  FILES:=$(LINUX_DIR)/net/ipv4/ip_vti.ko
  AUTOLOAD:=$(call AutoLoad,33,ip_vti)
endef

define KernelPackage/ip-vti/description
 Kernel modules for IP VTI (Virtual Tunnel Interface)
endef

$(eval $(call KernelPackage,ip-vti))


define KernelPackage/ip6-vti
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=IPv6 VTI (Virtual Tunnel Interface)
  DEPENDS:=@IPV6 +kmod-iptunnel +kmod-ip6-tunnel +kmod-ipsec6
  KCONFIG:=CONFIG_IPV6_VTI
  FILES:=$(LINUX_DIR)/net/ipv6/ip6_vti.ko
  AUTOLOAD:=$(call AutoLoad,33,ip6_vti)
endef

define KernelPackage/ip6-vti/description
 Kernel modules for IPv6 VTI (Virtual Tunnel Interface)
endef

$(eval $(call KernelPackage,ip6-vti))


define KernelPackage/xfrm-interface
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=IPsec XFRM Interface
  DEPENDS:=@IPV6 +kmod-ipsec4 +kmod-ipsec6
  KCONFIG:=CONFIG_XFRM_INTERFACE
  FILES:=$(LINUX_DIR)/net/xfrm/xfrm_interface.ko
  AUTOLOAD:=$(call AutoProbe,xfrm_interface)
endef

define KernelPackage/xfrm-interface/description
 Kernel module for XFRM interface support
endef

$(eval $(call KernelPackage,xfrm-interface))


define KernelPackage/iptunnel4
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=IPv4 tunneling
  HIDDEN:=1
  KCONFIG:= \
	CONFIG_INET_TUNNEL \
	CONFIG_NET_IPIP=m
  FILES:=$(LINUX_DIR)/net/ipv4/tunnel4.ko
  AUTOLOAD:=$(call AutoLoad,31,tunnel4)
endef

define KernelPackage/iptunnel4/description
 Kernel modules for IPv4 tunneling
endef

$(eval $(call KernelPackage,iptunnel4))


define KernelPackage/iptunnel6
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=IPv6 tunneling
  DEPENDS:=@IPV6
  KCONFIG:= \
	CONFIG_INET6_TUNNEL
  FILES:=$(LINUX_DIR)/net/ipv6/tunnel6.ko
  AUTOLOAD:=$(call AutoLoad,31,tunnel6)
endef

define KernelPackage/iptunnel6/description
 Kernel modules for IPv6 tunneling
endef

$(eval $(call KernelPackage,iptunnel6))


define KernelPackage/sit
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  DEPENDS:=@IPV6 +kmod-iptunnel +kmod-iptunnel4
  TITLE:=IPv6-in-IPv4 tunnel
  KCONFIG:=CONFIG_IPV6_SIT \
	CONFIG_IPV6_SIT_6RD=y
  FILES:=$(LINUX_DIR)/net/ipv6/sit.ko
  AUTOLOAD:=$(call AutoLoad,32,sit)
endef

define KernelPackage/sit/description
 Kernel modules for IPv6-in-IPv4 tunnelling
endef

$(eval $(call KernelPackage,sit))


define KernelPackage/fou
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=FOU and GUE decapsulation
  DEPENDS:= \
	+kmod-iptunnel \
	+kmod-udptunnel4 \
	+IPV6:kmod-udptunnel6
  KCONFIG:= \
	CONFIG_NET_FOU \
	CONFIG_NET_FOU_IP_TUNNELS=y
  FILES:=$(LINUX_DIR)/net/ipv4/fou.ko
  AUTOLOAD:=$(call AutoProbe,fou)
endef

define KernelPackage/fou/description
 Kernel module for FOU (Foo over UDP) and GUE (Generic UDP Encapsulation) tunnelling.
 Requires Kernel 3.18 or newer.
endef

$(eval $(call KernelPackage,fou))


define KernelPackage/fou6
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=FOU and GUE decapsulation over IPv6
  DEPENDS:= @IPV6 \
	+kmod-fou \
	+kmod-ip6-tunnel
  KCONFIG:= \
	CONFIG_IPV6_FOU \
	CONFIG_IPV6_FOU_TUNNEL
  FILES:=$(LINUX_DIR)/net/ipv6/fou6.ko
  AUTOLOAD:=$(call AutoProbe,fou6)
endef

define KernelPackage/fou6/description
 Kernel module for FOU (Foo over UDP) and GUE (Generic UDP Encapsulation) tunnelling over IPv6.
 Requires Kernel 3.18 or newer.
endef

$(eval $(call KernelPackage,fou6))


define KernelPackage/ip6-tunnel
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=IP-in-IPv6 tunnelling
  DEPENDS:=@IPV6 +kmod-iptunnel6
  KCONFIG:= CONFIG_IPV6_TUNNEL
  FILES:=$(LINUX_DIR)/net/ipv6/ip6_tunnel.ko
  AUTOLOAD:=$(call AutoLoad,32,ip6_tunnel)
endef

define KernelPackage/ip6-tunnel/description
 Kernel modules for IPv6-in-IPv6 and IPv4-in-IPv6 tunnelling
endef

$(eval $(call KernelPackage,ip6-tunnel))


define KernelPackage/gre
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=GRE support
  DEPENDS:=+kmod-iptunnel
  KCONFIG:=CONFIG_NET_IPGRE CONFIG_NET_IPGRE_DEMUX
  FILES:=$(LINUX_DIR)/net/ipv4/ip_gre.ko $(LINUX_DIR)/net/ipv4/gre.ko
  AUTOLOAD:=$(call AutoLoad,39,gre ip_gre)
endef

define KernelPackage/gre/description
 Generic Routing Encapsulation support
endef

$(eval $(call KernelPackage,gre))


define KernelPackage/gre6
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=GRE support over IPV6
  DEPENDS:=@IPV6 +kmod-iptunnel +kmod-ip6-tunnel +kmod-gre
  KCONFIG:=CONFIG_IPV6_GRE
  FILES:=$(LINUX_DIR)/net/ipv6/ip6_gre.ko
  AUTOLOAD:=$(call AutoLoad,39,ip6_gre)
endef

define KernelPackage/gre6/description
 Generic Routing Encapsulation support over IPv6
endef

$(eval $(call KernelPackage,gre6))


define KernelPackage/tun
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=Universal TUN/TAP driver
  KCONFIG:=CONFIG_TUN
  FILES:=$(LINUX_DIR)/drivers/net/tun.ko
  AUTOLOAD:=$(call AutoLoad,30,tun)
endef

define KernelPackage/tun/description
 Kernel support for the TUN/TAP tunneling device
endef

$(eval $(call KernelPackage,tun))


define KernelPackage/veth
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=Virtual ethernet pair device
  KCONFIG:=CONFIG_VETH
  FILES:=$(LINUX_DIR)/drivers/net/veth.ko
  AUTOLOAD:=$(call AutoLoad,30,veth)
endef

define KernelPackage/veth/description
 This device is a local ethernet tunnel. Devices are created in pairs.
 When one end receives the packet it appears on its pair and vice
 versa.
endef

$(eval $(call KernelPackage,veth))


define KernelPackage/vrf
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=Virtual Routing and Forwarding (Lite)
  DEPENDS:=@KERNEL_NET_L3_MASTER_DEV
  KCONFIG:=CONFIG_NET_VRF
  FILES:=$(LINUX_DIR)/drivers/net/vrf.ko
  AUTOLOAD:=$(call AutoLoad,30,vrf)
endef

define KernelPackage/vrf/description
 This option enables the support for mapping interfaces into VRF's. The
 support enables VRF devices.
endef

$(eval $(call KernelPackage,vrf))


define KernelPackage/slhc
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  HIDDEN:=1
  TITLE:=Serial Line Header Compression
  DEPENDS:=+kmod-lib-crc-ccitt
  KCONFIG:=CONFIG_SLHC
  FILES:=$(LINUX_DIR)/drivers/net/slip/slhc.ko
endef

$(eval $(call KernelPackage,slhc))


define KernelPackage/ppp
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=PPP modules
  DEPENDS:=+kmod-lib-crc-ccitt +kmod-slhc
  KCONFIG:= \
	CONFIG_PPP \
	CONFIG_PPP_ASYNC
  FILES:= \
	$(LINUX_DIR)/drivers/net/ppp/ppp_async.ko \
	$(LINUX_DIR)/drivers/net/ppp/ppp_generic.ko
  AUTOLOAD:=$(call AutoProbe,ppp_async)
endef

define KernelPackage/ppp/description
 Kernel modules for PPP support
endef

$(eval $(call KernelPackage,ppp))


define KernelPackage/ppp-synctty
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=PPP sync tty support
  DEPENDS:=kmod-ppp
  KCONFIG:=CONFIG_PPP_SYNC_TTY
  FILES:=$(LINUX_DIR)/drivers/net/ppp/ppp_synctty.ko
  AUTOLOAD:=$(call AutoProbe,ppp_synctty)
endef

define KernelPackage/ppp-synctty/description
 Kernel modules for PPP sync tty support
endef

$(eval $(call KernelPackage,ppp-synctty))


define KernelPackage/pppox
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=PPPoX helper
  DEPENDS:=kmod-ppp
  KCONFIG:=CONFIG_PPPOE
  FILES:=$(LINUX_DIR)/drivers/net/ppp/pppox.ko
endef

define KernelPackage/pppox/description
 Kernel helper module for PPPoE and PPTP support
endef

$(eval $(call KernelPackage,pppox))


define KernelPackage/pppoe
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=PPPoE support
  DEPENDS:=kmod-ppp +kmod-pppox
  KCONFIG:=CONFIG_PPPOE
  FILES:=$(LINUX_DIR)/drivers/net/ppp/pppoe.ko
  AUTOLOAD:=$(call AutoProbe,pppoe)
endef

define KernelPackage/pppoe/description
 Kernel module for PPPoE (PPP over Ethernet) support
endef

$(eval $(call KernelPackage,pppoe))


define KernelPackage/pppoa
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=PPPoA support
  DEPENDS:=kmod-ppp +kmod-atm
  KCONFIG:=CONFIG_PPPOATM CONFIG_ATM_DRIVERS=y
  FILES:=$(LINUX_DIR)/net/atm/pppoatm.ko
  AUTOLOAD:=$(call AutoLoad,40,pppoatm)
endef

define KernelPackage/pppoa/description
 Kernel modules for PPPoA (PPP over ATM) support
endef

$(eval $(call KernelPackage,pppoa))


define KernelPackage/pptp
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=PPtP support
  DEPENDS:=kmod-ppp +kmod-gre +kmod-pppox
  KCONFIG:=CONFIG_PPTP
  FILES:=$(LINUX_DIR)/drivers/net/ppp/pptp.ko
  AUTOLOAD:=$(call AutoProbe,pptp)
endef

$(eval $(call KernelPackage,pptp))


define KernelPackage/pppol2tp
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=PPPoL2TP support
  DEPENDS:=kmod-ppp +kmod-pppox +kmod-l2tp
  KCONFIG:=CONFIG_PPPOL2TP
  FILES:=$(LINUX_DIR)/net/l2tp/l2tp_ppp.ko
  AUTOLOAD:=$(call AutoProbe,l2tp_ppp)
endef

define KernelPackage/pppol2tp/description
  Kernel modules for PPPoL2TP (PPP over L2TP) support
endef

$(eval $(call KernelPackage,pppol2tp))


define KernelPackage/ipoa
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=IPoA support
  DEPENDS:=kmod-atm
  KCONFIG:=CONFIG_ATM_CLIP
  FILES:=$(LINUX_DIR)/net/atm/clip.ko
  AUTOLOAD:=$(call AutoProbe,clip)
endef

define KernelPackage/ipoa/description
  Kernel modules for IPoA (IP over ATM) support
endef

$(eval $(call KernelPackage,ipoa))


define KernelPackage/mppe
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=Microsoft PPP compression/encryption
  DEPENDS:=kmod-ppp +kmod-crypto-arc4 +kmod-crypto-sha1 +kmod-crypto-ecb
  KCONFIG:= \
	CONFIG_PPP_MPPE_MPPC \
	CONFIG_PPP_MPPE
  FILES:=$(LINUX_DIR)/drivers/net/ppp/ppp_mppe.ko
  AUTOLOAD:=$(call AutoProbe,ppp_mppe)
endef

define KernelPackage/mppe/description
 Kernel modules for Microsoft PPP compression/encryption
endef

$(eval $(call KernelPackage,mppe))


SCHED_MODULES = $(patsubst $(LINUX_DIR)/net/sched/%.ko,%,$(wildcard $(LINUX_DIR)/net/sched/*.ko))
SCHED_MODULES_CORE = sch_ingress sch_fq_codel sch_hfsc sch_htb sch_tbf cls_basic cls_fw cls_route cls_flow cls_tcindex cls_u32 em_u32 act_gact act_mirred act_skbedit cls_matchall
SCHED_MODULES_FILTER = $(SCHED_MODULES_CORE) act_connmark act_ctinfo sch_cake sch_netem sch_mqprio em_ipset cls_bpf cls_flower act_bpf act_vlan
SCHED_MODULES_EXTRA = $(filter-out $(SCHED_MODULES_FILTER),$(SCHED_MODULES))
SCHED_FILES = $(patsubst %,$(LINUX_DIR)/net/sched/%.ko,$(filter $(SCHED_MODULES_CORE),$(SCHED_MODULES)))
SCHED_FILES_EXTRA = $(patsubst %,$(LINUX_DIR)/net/sched/%.ko,$(SCHED_MODULES_EXTRA))

define KernelPackage/sched-core
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=Traffic schedulers
  KCONFIG:= \
	CONFIG_NET_SCHED=y \
	CONFIG_NET_SCH_HFSC \
	CONFIG_NET_SCH_HTB \
	CONFIG_NET_SCH_TBF \
	CONFIG_NET_SCH_INGRESS \
	CONFIG_NET_SCH_FQ_CODEL \
	CONFIG_NET_CLS=y \
	CONFIG_NET_CLS_ACT=y \
	CONFIG_NET_CLS_BASIC \
	CONFIG_NET_CLS_FLOW \
	CONFIG_NET_CLS_FW \
	CONFIG_NET_CLS_ROUTE4 \
	CONFIG_NET_CLS_TCINDEX \
	CONFIG_NET_CLS_U32 \
	CONFIG_NET_ACT_GACT \
	CONFIG_NET_ACT_MIRRED \
	CONFIG_NET_ACT_SKBEDIT \
	CONFIG_NET_CLS_MATCHALL \
	CONFIG_NET_EMATCH=y \
	CONFIG_NET_EMATCH_U32
  FILES:=$(SCHED_FILES)
  AUTOLOAD:=$(call AutoLoad,70, $(SCHED_MODULES_CORE))
endef

define KernelPackage/sched-core/description
 Core kernel scheduler support for IP traffic
endef

$(eval $(call KernelPackage,sched-core))


define KernelPackage/sched-cake
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=Cake fq_codel/blue derived shaper
  DEPENDS:=+kmod-sched-core
  KCONFIG:=CONFIG_NET_SCH_CAKE
  FILES:=$(LINUX_DIR)/net/sched/sch_cake.ko
  AUTOLOAD:=$(call AutoProbe,sch_cake)
endef

define KernelPackage/sched-cake/description
 Common Applications Kept Enhanced fq_codel/blue derived shaper
endef

$(eval $(call KernelPackage,sched-cake))

define KernelPackage/sched-flower
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=Flower traffic classifier
  DEPENDS:=+kmod-sched-core
  KCONFIG:=CONFIG_NET_CLS_FLOWER
  FILES:=$(LINUX_DIR)/net/sched/cls_flower.ko
  AUTOLOAD:=$(call AutoProbe, cls_flower)
endef

define KernelPackage/sched-flower/description
 Allows to classify packets based on a configurable combination of packet keys and masks.
endef

$(eval $(call KernelPackage,sched-flower))


define KernelPackage/sched-act-vlan
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=Traffic VLAN manipulation
  DEPENDS:=+kmod-sched-core
  KCONFIG:=CONFIG_NET_ACT_VLAN
  FILES:=$(LINUX_DIR)/net/sched/act_vlan.ko
  AUTOLOAD:=$(call AutoProbe, act_vlan)
endef

define KernelPackage/sched-act-vlan/description
 Allows to configure rules to push or pop vlan headers.
endef

$(eval $(call KernelPackage,sched-act-vlan))


define KernelPackage/sched-mqprio
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=Multi-queue priority scheduler (MQPRIO)
  DEPENDS:=+kmod-sched-core
  KCONFIG:=CONFIG_NET_SCH_MQPRIO
  FILES:=$(LINUX_DIR)/net/sched/sch_mqprio.ko
  AUTOLOAD:=$(call AutoProbe, sch_mqprio)
endef

define KernelPackage/sched-mqprio/description
  This scheduler allows QOS to be offloaded on NICs that have support for offloading QOS schedulers.
endef

$(eval $(call KernelPackage,sched-mqprio))

define KernelPackage/sched-connmark
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=Traffic shaper conntrack mark support
  DEPENDS:=+kmod-sched-core +kmod-ipt-core +kmod-ipt-conntrack-extra
  KCONFIG:=CONFIG_NET_ACT_CONNMARK
  FILES:=$(LINUX_DIR)/net/sched/act_connmark.ko
  AUTOLOAD:=$(call AutoLoad,71, act_connmark)
endef
$(eval $(call KernelPackage,sched-connmark))

define KernelPackage/sched-ctinfo
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=Traffic shaper ctinfo support
  DEPENDS:=+kmod-sched-core +kmod-ipt-core +kmod-ipt-conntrack-extra
  KCONFIG:=CONFIG_NET_ACT_CTINFO
  FILES:=$(LINUX_DIR)/net/sched/act_ctinfo.ko
  AUTOLOAD:=$(call AutoLoad,71, act_ctinfo)
endef
$(eval $(call KernelPackage,sched-ctinfo))

define KernelPackage/sched-ipset
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=Traffic shaper ipset support
  DEPENDS:=+kmod-sched-core +kmod-ipt-ipset
  KCONFIG:= \
	CONFIG_NET_EMATCH_IPSET
  FILES:= \
	$(LINUX_DIR)/net/sched/em_ipset.ko
  AUTOLOAD:=$(call AutoLoad,72,em_ipset)
endef

$(eval $(call KernelPackage,sched-ipset))


define KernelPackage/sched-bpf
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=Traffic shaper support for Berkeley Packet Filter
  KCONFIG:= \
	CONFIG_NET_CLS_BPF \
	CONFIG_NET_ACT_BPF
  FILES:= \
	$(LINUX_DIR)/net/sched/cls_bpf.ko \
	$(LINUX_DIR)/net/sched/act_bpf.ko
  AUTOLOAD:=$(call AutoLoad,72,cls_bpf act_bpf)
endef

$(eval $(call KernelPackage,sched-bpf))


define KernelPackage/bpf-test
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=Test Berkeley Packet Filter functionality
  KCONFIG:=CONFIG_TEST_BPF
  FILES:=$(LINUX_DIR)/lib/test_bpf.ko
endef

$(eval $(call KernelPackage,bpf-test))


define KernelPackage/sched
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=Extra traffic schedulers
  DEPENDS:=+kmod-sched-core +kmod-ipt-core +kmod-lib-crc32c
  KCONFIG:= \
	CONFIG_NET_SCH_CODEL \
	CONFIG_NET_SCH_DSMARK \
	CONFIG_NET_SCH_FIFO \
	CONFIG_NET_SCH_GRED \
	CONFIG_NET_SCH_MULTIQ \
	CONFIG_NET_SCH_PRIO \
	CONFIG_NET_SCH_RED \
	CONFIG_NET_SCH_SFQ \
	CONFIG_NET_SCH_TEQL \
	CONFIG_NET_SCH_FQ \
	CONFIG_NET_SCH_PIE \
	CONFIG_NET_ACT_POLICE \
	CONFIG_NET_ACT_IPT \
	CONFIG_NET_ACT_PEDIT \
	CONFIG_NET_ACT_SIMP \
	CONFIG_NET_ACT_CSUM \
	CONFIG_NET_EMATCH_CMP \
	CONFIG_NET_EMATCH_NBYTE \
	CONFIG_NET_EMATCH_META \
	CONFIG_NET_EMATCH_TEXT
  FILES:=$(SCHED_FILES_EXTRA)
  AUTOLOAD:=$(call AutoLoad,73, $(SCHED_MODULES_EXTRA))
endef

define KernelPackage/sched/description
 Extra kernel schedulers modules for IP traffic
endef

SCHED_TEQL_HOTPLUG:=hotplug-sched-teql.sh

define KernelPackage/sched/install
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_DATA) ./files/$(SCHED_TEQL_HOTPLUG) $(1)/etc/hotplug.d/iface/15-teql
endef

$(eval $(call KernelPackage,sched))


define KernelPackage/tcp-bbr
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=BBR TCP congestion control
  KCONFIG:=CONFIG_TCP_CONG_BBR
  FILES:=$(LINUX_DIR)/net/ipv4/tcp_bbr.ko
  AUTOLOAD:=$(call AutoProbe,tcp_bbr)
endef

define KernelPackage/tcp-bbr/description
 Kernel module for BBR (Bottleneck Bandwidth and RTT) TCP congestion
 control. It requires the fq ("Fair Queue") pacing packet scheduler.
 For kernel 4.13+, TCP internal pacing is implemented as fallback.
endef

TCP_BBR_SYSCTL_CONF:=sysctl-tcp-bbr.conf

define KernelPackage/tcp-bbr/install
	$(INSTALL_DIR) $(1)/etc/sysctl.d
	$(INSTALL_DATA) ./files/$(TCP_BBR_SYSCTL_CONF) $(1)/etc/sysctl.d/12-tcp-bbr.conf
endef

$(eval $(call KernelPackage,tcp-bbr))


define KernelPackage/tcp-hybla
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=TCP-Hybla congestion control algorithm
  KCONFIG:=CONFIG_TCP_CONG_HYBLA
  FILES:=$(LINUX_DIR)/net/ipv4/tcp_hybla.ko
  AUTOLOAD:=$(call AutoProbe,tcp_hybla)
endef

define KernelPackage/tcp-hybla/description
  TCP-Hybla is a sender-side only change that eliminates penalization of
  long-RTT, large-bandwidth connections, like when satellite legs are
  involved, especially when sharing a common bottleneck with normal
  terrestrial connections.
endef

$(eval $(call KernelPackage,tcp-hybla))


define KernelPackage/ax25
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=AX25 support
  DEPENDS:=+kmod-lib-crc16
  KCONFIG:= \
	CONFIG_HAMRADIO=y \
	CONFIG_AX25 \
	CONFIG_MKISS
  FILES:= \
	$(LINUX_DIR)/net/ax25/ax25.ko \
	$(LINUX_DIR)/drivers/net/hamradio/mkiss.ko
  AUTOLOAD:=$(call AutoLoad,80,ax25 mkiss)
endef

define KernelPackage/ax25/description
 Kernel modules for AX25 support
endef

$(eval $(call KernelPackage,ax25))


define KernelPackage/pktgen
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  DEPENDS:=@!TARGET_uml
  TITLE:=Network packet generator
  KCONFIG:=CONFIG_NET_PKTGEN
  FILES:=$(LINUX_DIR)/net/core/pktgen.ko
  AUTOLOAD:=$(call AutoLoad,99,pktgen)
endef

define KernelPackage/pktgen/description
  Kernel modules for the Network Packet Generator
endef

$(eval $(call KernelPackage,pktgen))

define KernelPackage/l2tp
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=Layer Two Tunneling Protocol (L2TP)
  DEPENDS:= \
	+kmod-udptunnel4 \
	+IPV6:kmod-udptunnel6
  KCONFIG:=CONFIG_L2TP \
	CONFIG_L2TP_V3=y \
	CONFIG_L2TP_DEBUGFS=n
  FILES:=$(LINUX_DIR)/net/l2tp/l2tp_core.ko \
	$(LINUX_DIR)/net/l2tp/l2tp_netlink.ko
  AUTOLOAD:=$(call AutoLoad,32,l2tp_core l2tp_netlink)
endef

define KernelPackage/l2tp/description
 Kernel modules for L2TP V3 Support
endef

$(eval $(call KernelPackage,l2tp))


define KernelPackage/l2tp-eth
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=L2TP ethernet pseudowire support for L2TPv3
  DEPENDS:=+kmod-l2tp
  KCONFIG:=CONFIG_L2TP_ETH
  FILES:=$(LINUX_DIR)/net/l2tp/l2tp_eth.ko
  AUTOLOAD:=$(call AutoLoad,33,l2tp_eth)
endef

define KernelPackage/l2tp-eth/description
 Kernel modules for L2TP ethernet pseudowire support for L2TPv3
endef

$(eval $(call KernelPackage,l2tp-eth))

define KernelPackage/l2tp-ip
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=L2TP IP encapsulation for L2TPv3
  DEPENDS:=+kmod-l2tp
  KCONFIG:=CONFIG_L2TP_IP
  FILES:= \
	$(LINUX_DIR)/net/l2tp/l2tp_ip.ko \
	$(if $(CONFIG_IPV6),$(LINUX_DIR)/net/l2tp/l2tp_ip6.ko)
  AUTOLOAD:=$(call AutoLoad,33,l2tp_ip $(if $(CONFIG_IPV6),l2tp_ip6))
endef

define KernelPackage/l2tp-ip/description
 Kernel modules for L2TP IP encapsulation for L2TPv3
endef

$(eval $(call KernelPackage,l2tp-ip))


define KernelPackage/sctp
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=SCTP protocol kernel support
  KCONFIG:=\
     CONFIG_IP_SCTP \
     CONFIG_SCTP_DBG_MSG=n \
     CONFIG_SCTP_DBG_OBJCNT=n \
     CONFIG_SCTP_HMAC_NONE=n \
     CONFIG_SCTP_HMAC_SHA1=n \
     CONFIG_SCTP_HMAC_MD5=y \
     CONFIG_SCTP_COOKIE_HMAC_SHA1=n \
     CONFIG_SCTP_COOKIE_HMAC_MD5=y \
     CONFIG_SCTP_DEFAULT_COOKIE_HMAC_NONE=n \
     CONFIG_SCTP_DEFAULT_COOKIE_HMAC_SHA1=n \
     CONFIG_SCTP_DEFAULT_COOKIE_HMAC_MD5=y
  FILES:= $(LINUX_DIR)/net/sctp/sctp.ko
  AUTOLOAD:= $(call AutoLoad,32,sctp)
  DEPENDS:=+kmod-lib-crc32c +kmod-crypto-md5 +kmod-crypto-hmac \
    +LINUX_5_15:kmod-udptunnel4 +LINUX_5_15:kmod-udptunnel6
endef

define KernelPackage/sctp/description
 Kernel modules for SCTP protocol support
endef

$(eval $(call KernelPackage,sctp))


define KernelPackage/netem
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=Network emulation functionality
  DEPENDS:=+kmod-sched
  KCONFIG:=CONFIG_NET_SCH_NETEM
  FILES:=$(LINUX_DIR)/net/sched/sch_netem.ko
  AUTOLOAD:=$(call AutoLoad,99,netem)
endef

define KernelPackage/netem/description
  Kernel modules for emulating the properties of wide area networks
endef

$(eval $(call KernelPackage,netem))

define KernelPackage/slip
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  DEPENDS:=+kmod-slhc
  TITLE:=SLIP modules
  KCONFIG:= \
       CONFIG_SLIP \
       CONFIG_SLIP_COMPRESSED=y \
       CONFIG_SLIP_SMART=y \
       CONFIG_SLIP_MODE_SLIP6=y

  FILES:= \
       $(LINUX_DIR)/drivers/net/slip/slip.ko
  AUTOLOAD:=$(call AutoLoad,30,slip)
endef

define KernelPackage/slip/description
 Kernel modules for SLIP support
endef

$(eval $(call KernelPackage,slip))

define KernelPackage/dnsresolver
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=In-kernel DNS Resolver
  KCONFIG:= CONFIG_DNS_RESOLVER
  FILES:=$(LINUX_DIR)/net/dns_resolver/dns_resolver.ko
  AUTOLOAD:=$(call AutoLoad,30,dns_resolver)
endef

$(eval $(call KernelPackage,dnsresolver))

define KernelPackage/rxrpc
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=AF_RXRPC support
  DEPENDS:= \
    +kmod-crypto-manager \
    +kmod-crypto-pcbc \
    +kmod-crypto-fcrypt \
    +kmod-udptunnel4 \
    +IPV6:kmod-udptunnel6
  HIDDEN:=1
  KCONFIG:= \
	CONFIG_AF_RXRPC \
	CONFIG_RXKAD=m \
	CONFIG_AF_RXRPC_DEBUG=n
  FILES:= \
	$(LINUX_DIR)/net/rxrpc/rxrpc.ko
  AUTOLOAD:=$(call AutoLoad,30,rxrpc.ko)
endef

define KernelPackage/rxrpc/description
  Kernel support for AF_RXRPC; required for AFS client
endef

$(eval $(call KernelPackage,rxrpc))

define KernelPackage/mpls
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=MPLS support
  DEPENDS:=+kmod-iptunnel
  KCONFIG:= \
	CONFIG_MPLS=y \
	CONFIG_LWTUNNEL=y \
	CONFIG_LWTUNNEL_BPF=n \
	CONFIG_NET_MPLS_GSO=m \
	CONFIG_MPLS_ROUTING=m \
	CONFIG_MPLS_IPTUNNEL=m
  FILES:= \
	$(LINUX_DIR)/net/mpls/mpls_gso.ko \
	$(LINUX_DIR)/net/mpls/mpls_iptunnel.ko \
	$(LINUX_DIR)/net/mpls/mpls_router.ko
  AUTOLOAD:=$(call AutoLoad,30,mpls_router mpls_iptunnel mpls_gso)
endef

define KernelPackage/mpls/description
  Kernel support for MPLS
endef

$(eval $(call KernelPackage,mpls))

define KernelPackage/9pnet
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=Plan 9 Resource Sharing Support (9P2000)
  DEPENDS:=@VIRTIO_SUPPORT
  KCONFIG:= \
	CONFIG_NET_9P \
	CONFIG_NET_9P_DEBUG=n \
	CONFIG_NET_9P_XEN=n \
	CONFIG_NET_9P_VIRTIO
  FILES:= \
	$(LINUX_DIR)/net/9p/9pnet.ko \
	$(LINUX_DIR)/net/9p/9pnet_virtio.ko
  AUTOLOAD:=$(call AutoLoad,29,9pnet 9pnet_virtio)
endef

define KernelPackage/9pnet/description
  Kernel support support for
  Plan 9 resource sharing via the 9P2000 protocol.
endef

$(eval $(call KernelPackage,9pnet))


define KernelPackage/nlmon
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=Virtual netlink monitoring device
  KCONFIG:=CONFIG_NLMON
  FILES:=$(LINUX_DIR)/drivers/net/nlmon.ko
  AUTOLOAD:=$(call AutoProbe,nlmon)
endef

define KernelPackage/nlmon/description
  Kernel module which adds a monitoring device for netlink.
endef

$(eval $(call KernelPackage,nlmon))


define KernelPackage/mdio
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=MDIO (clause 45) PHY support
  KCONFIG:=CONFIG_MDIO
  HIDDEN:=1
  FILES:=$(LINUX_DIR)/drivers/net/mdio.ko
  AUTOLOAD:=$(call AutoLoad,32,mdio)
endef

define KernelPackage/mdio/description
 Kernel modules for MDIO (clause 45) PHY support
endef

$(eval $(call KernelPackage,mdio))

define KernelPackage/macsec
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=IEEE 802.1AE MAC-level encryption (MAC)
  DEPENDS:=+kmod-crypto-gcm
  KCONFIG:=CONFIG_MACSEC
  FILES:=$(LINUX_DIR)/drivers/net/macsec.ko
  AUTOLOAD:=$(call AutoLoad,13,macsec)
endef

define KernelPackage/macsec/description
 MACsec is an encryption standard for Ethernet.
endef

$(eval $(call KernelPackage,macsec))


define KernelPackage/netlink-diag
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=Netlink diag support for ss utility
  KCONFIG:=CONFIG_NETLINK_DIAG
  FILES:=$(LINUX_DIR)/net/netlink/netlink_diag.ko
  AUTOLOAD:=$(call AutoLoad,31,netlink-diag)
endef

define KernelPackage/netlink-diag/description
 Netlink diag is a module made for use with iproute2's ss utility
endef

$(eval $(call KernelPackage,netlink-diag))


define KernelPackage/inet-diag
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=INET diag support for ss utility
  KCONFIG:= \
	CONFIG_INET_DIAG \
	CONFIG_INET_TCP_DIAG \
	CONFIG_INET_UDP_DIAG \
	CONFIG_INET_RAW_DIAG \
	CONFIG_INET_DIAG_DESTROY=n
  FILES:= \
	$(LINUX_DIR)/net/ipv4/inet_diag.ko \
	$(LINUX_DIR)/net/ipv4/tcp_diag.ko \
	$(LINUX_DIR)/net/ipv4/udp_diag.ko \
	$(LINUX_DIR)/net/ipv4/raw_diag.ko
  AUTOLOAD:=$(call AutoLoad,31,inet_diag tcp_diag udp_diag raw_diag)
endef

define KernelPackage/inet-diag/description
Support for INET (TCP, DCCP, etc) socket monitoring interface used by
native Linux tools such as ss.
endef

$(eval $(call KernelPackage,inet-diag))


define KernelPackage/wireguard
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=WireGuard secure network tunnel
  DEPENDS:= \
	  +kmod-crypto-lib-chacha20poly1305 \
	  +kmod-crypto-lib-curve25519 \
	  +kmod-udptunnel4 \
	  +IPV6:kmod-udptunnel6
  KCONFIG:= \
	  CONFIG_WIREGUARD \
	  CONFIG_WIREGUARD_DEBUG=n
  FILES:=$(LINUX_DIR)/drivers/net/wireguard/wireguard.ko
  AUTOLOAD:=$(call AutoProbe,wireguard)
endef

define KernelPackage/wireguard/description
  WireGuard is a novel VPN that runs inside the Linux Kernel and utilizes
  state-of-the-art cryptography. It aims to be faster, simpler, leaner, and
  more useful than IPSec, while avoiding the massive headache. It intends to
  be considerably more performant than OpenVPN.  WireGuard is designed as a
  general purpose VPN for running on embedded interfaces and super computers
  alike, fit for many different circumstances. It uses UDP.
endef

$(eval $(call KernelPackage,wireguard))


define KernelPackage/netconsole
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=Network console logging support
  KCONFIG:=CONFIG_NETCONSOLE \
	  CONFIG_NETCONSOLE_DYNAMIC=n
  FILES:=$(LINUX_DIR)/drivers/net/netconsole.ko
  AUTOLOAD:=$(call AutoProbe,netconsole)
endef

define KernelPackage/netconsole/description
  Network console logging support.
endef

$(eval $(call KernelPackage,netconsole))


define KernelPackage/qrtr
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=Qualcomm IPC Router support
  HIDDEN:=1
  DEPENDS:=@(LINUX_5_15||LINUX_6_0)
  KCONFIG:=CONFIG_QRTR
  FILES:= \
  $(LINUX_DIR)/net/qrtr/qrtr.ko \
  $(LINUX_DIR)/net/qrtr/ns.ko
  AUTOLOAD:=$(call AutoProbe,qrtr)
endef

define KernelPackage/qrtr/description
 Qualcomm IPC Router support
endef

$(eval $(call KernelPackage,qrtr))

define KernelPackage/qrtr-tun
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=TUN device for Qualcomm IPC Router
  DEPENDS:=+kmod-qrtr
  KCONFIG:=CONFIG_QRTR_TUN
  FILES:= $(LINUX_DIR)/net/qrtr/qrtr-tun.ko
  AUTOLOAD:=$(call AutoProbe,qrtr-tun)
endef

define KernelPackage/qrtr-tun/description
 TUN device for Qualcomm IPC Router
endef

$(eval $(call KernelPackage,qrtr-tun))

define KernelPackage/qrtr-smd
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=SMD IPC Router channels
  DEPENDS:=+kmod-qrtr @(TARGET_ipq60xx||TARGET_ipq807x)
  KCONFIG:=CONFIG_QRTR_SMD
  FILES:= $(LINUX_DIR)/net/qrtr/qrtr-smd.ko
  AUTOLOAD:=$(call AutoProbe,qrtr-smd)
endef

define KernelPackage/qrtr-smd/description
 SMD IPC Router channels
endef

$(eval $(call KernelPackage,qrtr-smd))

define KernelPackage/qrtr-mhi
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=MHI IPC Router channels
  DEPENDS:=+kmod-mhi +kmod-qrtr
  KCONFIG:=CONFIG_QRTR_MHI
  FILES:= $(LINUX_DIR)/net/qrtr/qrtr-mhi.ko
  AUTOLOAD:=$(call AutoProbe,qrtr-mhi)
endef

define KernelPackage/qrtr-mhi/description
 MHI IPC Router channels
endef

$(eval $(call KernelPackage,qrtr-mhi))


define KernelPackage/mptcp
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=MultiPath TCP support
  KCONFIG:=CONFIG_MPTCP@ge5.6=y
  AUTOLOAD:=$(call AutoProbe,mptcp)
endef

define KernelPackage/mptcp/description
 MPTCP is a module made for MultiPath TCP support
endef

$(eval $(call KernelPackage,mptcp))


define KernelPackage/mptcp_ipv6
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=MultiPath TCP IPv6 support
  DEPENDS:=@IPV6 +kmod-mptcp
  KCONFIG:=CONFIG_MPTCP_IPV6@ge5.6=y
  AUTOLOAD:=$(call AutoProbe,mptcp_ipv6)
endef

define KernelPackage/mptcp_ipv6/description
 MPTCP_IPV6 is a module made for MultiPath TCP IPv6 support
endef

$(eval $(call KernelPackage,mptcp_ipv6))


define KernelPackage/inet-mptcp-diag
  SUBMENU:=$(NETWORK_SUPPORT_MENU)
  TITLE:=INET diag support for MultiPath TCP
  DEPENDS:=kmod-mptcp +kmod-inet-diag
  KCONFIG:= CONFIG_INET_MPTCP_DIAG@ge5.6
  FILES:= $(LINUX_DIR)/net/mptcp/mptcp_diag.ko@ge5.6
  AUTOLOAD:=$(call AutoProbe,mptcp_diag)
endef

define KernelPackage/inet-mptcp-diag/description
Support for INET (MultiPath TCP) socket monitoring interface used by
native Linux tools such as ss.
endef

$(eval $(call KernelPackage,inet-mptcp-diag))
