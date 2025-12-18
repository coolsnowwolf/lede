# SPDX-License-Identifier: GPL-2.0-only
#
# Makefile for the siflower soc network device drivers.
#

sf_dpns-objs := sf_dpns.o sf_dpns_debugfs.o sf_dpns_tmu.o sf_dpns_se.o
obj-$(CONFIG_NET_SIFLOWER_ETH_DPNS) += sf_dpns.o

obj-$(CONFIG_NET_SIFLOWER_ETH_XGMAC) += sfxgmac.o
obj-$(CONFIG_NET_SIFLOWER_ETH_DMA) += sfxgmac-dma.o
obj-$(CONFIG_NET_SIFLOWER_ETH_XPCS) += sfxpcs.o
