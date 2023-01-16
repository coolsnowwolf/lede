#
# Makefile for Shortcut FE.
#

SFE_BASE_OBJS := sfe.o sfe_init.o
SFE_IPV4_OBJS := sfe_ipv4.o sfe_ipv4_udp.o sfe_ipv4_tcp.o sfe_ipv4_icmp.o
SFE_IPV6_OBJS := sfe_ipv6.o sfe_ipv6_udp.o sfe_ipv6_tcp.o sfe_ipv6_icmp.o
SFE_PPPOE_OBJS := sfe_pppoe.o

obj-m += qca-nss-sfe.o

#
# Base files
#
qca-nss-sfe-objs := $(SFE_BASE_OBJS)

#
# IPv4 files
#
qca-nss-sfe-objs += $(SFE_IPV4_OBJS)
qca-nss-sfe-objs += $(SFE_PPPOE_OBJS)

ifdef SFE_SUPPORT_IPV6
qca-nss-sfe-objs += $(SFE_IPV6_OBJS)
ccflags-y += -DSFE_SUPPORT_IPV6
endif

ifdef SFE_PROCESS_LOCAL_OUT
ccflags-y += -DSFE_PROCESS_LOCAL_OUT
endif
ccflags-y += -Werror -Wall
