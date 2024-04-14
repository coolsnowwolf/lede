obj-m := GobiNet.o
GobiNet-objs := GobiUSBNet.o QMIDevice.o QMI.o

PWD := $(shell pwd)
OUTPUTDIR=/lib/modules/`uname -r`/kernel/drivers/net/usb/

ifeq ($(ARCH),)
ARCH := $(shell uname -m)
endif
ifeq ($(CROSS_COMPILE),)
CROSS_COMPILE :=
endif
ifeq ($(KDIR),)
KDIR := /lib/modules/$(shell uname -r)/build
ifeq ($(ARCH),i686)
ifeq ($(wildcard $KDIR/arch/$ARCH),)
ARCH=i386
endif
endif
endif

$(shell rm -rf usbnet.h)
ifneq ($(wildcard $(KDIR)/drivers/usb/net/usbnet.h),)
$(shell ln -s $(KDIR)/drivers/usb/net/usbnet.h usbnet.h)
endif
ifneq ($(wildcard $(KDIR)/drivers/net/usb/usbnet.h),)
$(shell ln -s $(KDIR)/drivers/net/usb/usbnet.h usbnet.h)
endif

default:
	ln -sf makefile Makefile
	$(MAKE) ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} -C $(KDIR) M=$(PWD) modules

install: default
	mkdir -p $(OUTPUTDIR)
	cp -f GobiNet.ko $(OUTPUTDIR)
	depmod
	modprobe -r GobiNet
	modprobe GobiNet

clean:
	rm -rf Makefile usbnet.h
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions Module.* modules.order
