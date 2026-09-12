
# SPDX-License-Identifier: GPL-2.0
# Copyright(c) 2022 - 2025 Phytium Technology Co., Ltd.
#
# Makefile for the Phytium network device drivers.
#

obj-$(CONFIG_PHYTMAC) += phytmac.o

phytmac-objs := phytmac_main.o phytmac_ethtool.o phytmac_v1.o phytmac_v2.o
phytmac-$(CONFIG_PHYTMAC_ENABLE_PTP) += phytmac_ptp.o

obj-$(CONFIG_PHYTMAC_PLATFORM) += phytmac-platform.o
phytmac-platform-objs := phytmac_platform.o

obj-$(CONFIG_PHYTMAC_PCI) += phytmac-pci.o
phytmac-pci-objs := phytmac_pci.o
