#
# Makefile for Lantiq platform
#

COMMON_DIR	:= ../common

EXTRA_CFLAGS	+= -DOUTBOUND_HW_SWAP

default: all
qdpc-host-objs	:=  $(COMMON_DIR)/qdpc_ring.o \
                    $(COMMON_DIR)/qdpc_net.o  \
                    $(COMMON_DIR)/qdpc_pcie.o \
                    $(COMMON_DIR)/qdpc_init.o

obj-m		:=  qdpc-host.o

all:
	$(MAKE) -C $(kernel_source) SUBDIRS=`pwd`  modules;

clean:
	rm -rf *.o  *~  core  .depend  .*.cmd   *.ko   *.mod.c  .tmp_versions  Module.symvers
	rm -rf $(COMMON_DIR)/.*.cmd $(COMMON_DIR)/.tmp_versions
	rm -rf Module.markers  modules.order $(qdpc-host-objs)

