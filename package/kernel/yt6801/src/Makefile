# SPDX-License-Identifier: GPL-2.0-only
################################################################################
#
# Copyright (c) 2023 Motorcomm, Inc.
# Motorcomm Confidential and Proprietary.
#
# This is Motorcomm NIC driver relevant files. Please don't copy, modify,
# distribute without commercial permission.
#
################################################################################

SUBARCH := $(shell uname -m | sed -e s/i.86/x86/ -e s/x86_64/x86/ -e s/sun4u/sparc64/ -e s/arm.*/arm/ -e s/sa110/arm/ -e s/s390x/s390/ -e s/parisc64/parisc/ -e s/ppc.*/powerpc/ -e s/mips.*/mips/ -e s/sh[234].*/sh/ -e s/aarch64.*/arm64/ -e s/riscv.*/riscv/)
CURARCH ?= $(SUBARCH)
ARCH ?= $(SUBARCH)
CROSS_COMPILE ?= 
#CONFIG_MODULE_SIG=n
PWD :=$(shell pwd)
EXTRA_CFLAGS = -Wall -g -I$(CURDIR) -I$(subst fuxi-linux-release-package/module_fuxi/src,common,$(PWD)) -I$(PWD)
EXTRA_CFLAGS += -DFXGMAC_DEBUG

KSRC_BASE = /lib/modules/$(shell uname -r)
KSRC =  $(KSRC_BASE)/build
#KDST = /lib/modules/$(shell uname -r)/kernel/drivers/net/ethernet/motorcomm/
KDST = kernel/drivers/net/ethernet/motorcomm
ko_dir = $(KSRC_BASE)/$(KDST)/
KFILE = yt6801
ko_full = $(ko_dir)$(KFILE).ko

yt6801-objs :=  fuxi-gmac-common.o fuxi-gmac-desc.o fuxi-gmac-ethtool.o fuxi-gmac-hw.o fuxi-gmac-net.o fuxi-gmac-pci.o fuxi-gmac-phy.o fuxi-efuse.o  fuxi-gmac-ioctl.o
obj-m += yt6801.o
modules:
	make -C $(KSRC) M=$(PWD) modules

install:
	@echo "KFILE: " $(KFILE)
	@echo "KDST: " $(KDST)
	make -C $(KSRC) M=$(PWD) INSTALL_MOD_DIR=$(KDST) modules_install
	sudo ls -l $(ko_dir)
	depmod $(shell uname -r)
	modprobe $(KFILE)
	@file $(ko_full)
	@echo install done.
	@modinfo $(ko_full)

uninstall:
	sudo ls -l $(ko_dir)
	sudo rm $(ko_full)
	sudo ls -l $(ko_dir)

clean:
	make -C $(KSRC) M=$(PWD) clean

.PHONY:modules install uninstall clean
