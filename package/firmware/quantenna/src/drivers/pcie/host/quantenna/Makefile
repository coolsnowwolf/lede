#
# Makefile for Quantenna RC paltform
#
#

PCIE_HOST_BASE	= ../drivers/pcie/host/

EXTRA_CFLAGS	+= -Wall  \
		   -I../include \
		   -I$(src)/../../../drivers \
		   -I$(src)/../../../include \
		   -I$(src)/../../include \
		   -I$(src)/../common \
		   -I$(src)/../quantenna

EXTRA_CFLAGS    += -mlong-calls

ifeq (${PCIE_HOST_CRUMBS},1)
EXTRA_CFLAGS += -finstrument-functions
endif

#EXTRA_CFLAGS	+= -DDEBUG

ifneq ($(KERNELRELEASE),)
COMMON_DIR	:= ../common
qdpc-host-objs   :=  $(COMMON_DIR)/qdpc_ring.o \
		    $(COMMON_DIR)/qdpc_net.o  \
		    $(COMMON_DIR)/qdpc_pcie.o \
		    $(COMMON_DIR)/qdpc_init.o

obj-m           :=  qdpc-host.o

else

KERNELDIR	?= ../../../../linux
INSTALL		= INSTALL_MOD_PATH=../linux/modules
CROSS		= ARCH=arc CROSS_COMPILE=/usr/local/ARC/gcc/bin/arc-linux-uclibc-
PWD		:= $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) $(CROSS) M=$(PWD) modules

install:
	$(MAKE) -C $(KERNELDIR) $(CROSS) $(INSTALL) M=$(PWD) modules_install
	
endif

clean:
	rm -rf *.o  *~  core  .depend  .*.cmd  *.ko  *.mod.c  .tmp_versions  Module.symvers  modules.order
	rm -rf $(COMMON_DIR)/.*.cmd $(COMMON_DIR)/.tmp_versions $(COMMON_DIR)/*.o 

depend .depend dep:
	$(CC) $(CFLAGS) -M *.c > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
