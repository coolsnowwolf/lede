#
# Makefile for Shortcut FE.
#

obj-m += shortcut-fe.o

ifdef SFE_SUPPORT_IPV6
obj-m += shortcut-fe-ipv6.o
endif

obj-m += shortcut-fe-cm.o

shortcut-fe-objs := \
	sfe_ipv4.o

ifdef SFE_SUPPORT_IPV6
shortcut-fe-ipv6-objs := \
	sfe_ipv6.o
endif

shortcut-fe-cm-objs := \
	sfe_cm.o

