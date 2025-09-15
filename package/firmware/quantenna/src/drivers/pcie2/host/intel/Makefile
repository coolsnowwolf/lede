#
# Makefile for Intel platform
#

EXTRA_CFLAGS	+= -Wall		\
		   -I$(src)		\
		   -I$(src)/../../include \
		   -I$(src)/../common

EXTRA_CFLAGS	+= -DRC_TXDONE_TIMER -DQTN_WAKEQ_SUPPORT
ifneq ($(CONFIG_HOTPLUG_PCI_PCIE),)
EXTRA_CFLAGS    += -DPCIE_HOTPLUG_SUPPORTED
endif

KVERSION = $(shell uname -r)

default: all

COMMON_DIR	:= ../common
qdpc-host-objs   := $(COMMON_DIR)/qdpc_init.o $(COMMON_DIR)/qdpc_pcie.o $(COMMON_DIR)/topaz_vnet.o qdpc_platform.o
obj-m           :=  qdpc-host.o

qdpc_host.o: $(qdpc-host-objs)
	ld -r $^ -o $@

all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean
	rm -rf $(COMMON_DIR)/.*.cmd $(COMMON_DIR)/.tmp_versions
	rm -rf Module.markers  modules.order *~ $(qdpc-host-objs)

