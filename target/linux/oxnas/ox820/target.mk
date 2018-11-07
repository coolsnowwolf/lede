SUBTARGET:=ox820
BOARDNAME:=OX820/NAS782x
CPU_TYPE:=mpcore
FEATURES+=nand pci pcie ubifs usb

DEFAULT_PACKAGES += \
	uboot-oxnas-ox820


define Target/Description
    Oxford/PLXTECH OX820/NAS782x
endef