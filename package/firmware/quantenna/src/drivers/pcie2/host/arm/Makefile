#
# Makefile for arm platform
#

EXTRA_CFLAGS	+= -Wall -Wno-deprecated-declarations	\
		   -I$(src)		\
		   -I$(src)/../../include \
		   -I$(src)/../common $(DNI_KMOD_CFLAGS)

EXTRA_CFLAGS    += -DQTN_TX_SKBQ_SUPPORT -DQTN_WAKEQ_SUPPORT

PWD	:= $(shell pwd)

default: all

COMMON_DIR	:= ../common
qdpc-host-objs   := $(COMMON_DIR)/qdpc_init.o $(COMMON_DIR)/qdpc_pcie.o $(COMMON_DIR)/topaz_vnet.o qdpc_platform.o
obj-m           :=  qdpc-host.o

qdpc_host.o: $(qdpc-host-objs)
	ld -r $^ -o $@

all:
	make -C $(KERNELDIR) $(CROSS) M=$(PWD) modules


clean:
	rm -rf $(COMMON_DIR)/.*.cmd $(COMMON_DIR)/.tmp_versions
	rm -rf Module.markers  Module.symvers modules.order *~ $(qdpc-host-objs) *.o *.ko *.mod.o *.mod.c

