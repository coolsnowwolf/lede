#
# Quantenna Communications Inc. Driver Makefile
#
# Author:
#
EXTRA_CFLAGS	+= -Wall \
		   -I../include \
		   -I../common \
		   -I../drivers/include/shared \
		   -I../drivers/include/kernel

EXTRA_CFLAGS    += -I$(M)/pcie/include
EXTRA_CFLAGS    += -mlong-calls

EXTRA_CFLAGS    += -DUSE_EMAC_DMA
EXTRA_CFLAGS    += -DQDPC_USE_DMA_RX_DONE_INTR
EXTRA_CFLAGS    += -DQDPC_DSPIO
EXTRA_CFLAGS    += -DQDPC_PHY_CROSS_MODE
#EXTRA_CFLAGS    += -DCVM_HOST
#EXTRA_CFLAGS    += -DDEBUG

ifeq (${PCIE_TARGET_CRUMBS},1)
EXTRA_CFLAGS += -finstrument-functions
endif

ifneq ($(KERNELRELEASE),)
qdpc-pcie-objs   :=  qdpc_ring.o \
		    qdpc_emac.o  \
		    qdpc_net.o  \
		    qdpc_pcie.o \
		    qdpc_init.o

obj-m           :=  qdpc-pcie.o
else

KERNELDIR	?= ../../../linux
INSTALL		= INSTALL_MOD_PATH=../linux/modules
CROSS		= ARCH=arc CROSS_COMPILE=/usr/local/ARC/gcc/bin/arc-linux-uclibc-
PWD		:= $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) $(CROSS) M=$(PWD) modules

install:
	$(MAKE) -C $(KERNELDIR) $(CROSS) $(INSTALL) M=$(PWD) modules_install
	
endif

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions Module.symvers modules.order

depend .depend dep:
	$(CC) $(CFLAGS) -M *.c > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
