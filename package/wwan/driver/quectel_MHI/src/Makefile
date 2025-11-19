ccflags-y += -Wno-error=declaration-after-statement
obj-m += pcie_mhi.o
pcie_mhi-objs := core/mhi_init.o core/mhi_main.o core/mhi_pm.o core/mhi_boot.o core/mhi_dtr.o  controllers/mhi_qti.o
pcie_mhi-objs += devices/mhi_uci.o

ifeq (1,1)
pcie_mhi-objs += devices/mhi_netdev_quectel.o
else
pcie_mhi-objs += devices/mhi_netdev.o
pcie_mhi-objs += devices/rmnet_handler.o
endif

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

pcie_mhi: clean
	$(MAKE) ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} -C $(KDIR) M=$(PWD) modules
	#cp pcie_mhi.ko /tftpboot/

clean:
	$(MAKE) ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} -C $(KDIR) M=$(PWD) clean
	find . -name *.o.ur-safe | xargs rm -f

install: pcie_mhi
	sudo cp pcie_mhi.ko /lib/modules/${shell uname -r}/kernel/drivers/pci/
	sudo depmod
