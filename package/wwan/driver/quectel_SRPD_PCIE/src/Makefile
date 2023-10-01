#
# Makefile for the sprd staging modem files
#
EXTRA_CFLAGS += -Wno-error -Wno-packed-bitfield-compat
ccflags-y += -DCONFIG_SPRD_PCIE_EP_DEVICE -DCONFIG_SPRD_SIPA -DCONFIG_SPRD_ETHERNET
obj-m += sprd_pcie.o
sprd_pcie-objs := pcie/sprd_pcie_ep_device.o pcie/pcie_host_resource.o pcie/sprd_pcie_quirks.o sipc/sipc.o sipc/sblock.o sipc/sbuf.o \
                  sipc/sipc_debugfs.o sipc/smem.o sipc/smsg.o sipc/spipe.o sipc/spool.o power_manager/power_manager.o \
		  sipa/sipa_core.o sipa/sipa_eth.o sipa/sipa_nic.o sipa/sipa_skb_send.o sipa/sipa_skb_recv.o sipa/sipa_dummy.o  sipa/sipa_debugfs.o sipa/sipa_dele_cmn.o \
		  sipa/sipa_phy_v0/sipa_fifo_irq_hal.o sipa/sipa_phy_v0/sipa_common_fifo_hal.o

PWD := $(shell pwd)
ifeq ($(ARCH),)
ARCH := $(shell uname -m)
endif
ifeq ($(CROSS_COMPILE),)
CROSS_COMPILE :=
endif
ifeq ($(KDIR),)
KDIR := /lib/modules/$(shell uname -r)/build
endif

sprd_pcie: clean
	$(MAKE) ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} -C $(KDIR) M=$(PWD) modules
	#cp sprd_pcie.ko /tftpboot/

clean:
	$(MAKE) ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} -C $(KDIR) M=$(PWD) clean
	find . -name *.o.ur-safe | xargs rm -f

install: sprd_pcie
	sudo cp sprd_pcie.ko /lib/modules/${shell uname -r}/kernel/drivers/pci/
	sudo depmod
