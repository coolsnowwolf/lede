include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=airoha
BOARDNAME:=Airoha ARM
SUBTARGETS:=en7523 an7581
FEATURES:=dt squashfs nand ramdisk gpio

KERNEL_PATCHVER:=6.6

include $(INCLUDE_DIR)/target.mk

$(eval $(call BuildTarget))
