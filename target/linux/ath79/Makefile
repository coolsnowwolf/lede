include $(TOPDIR)/rules.mk

ARCH:=mips
BOARD:=ath79
BOARDNAME:=Atheros ATH79 (DTS)
CPU_TYPE:=24kc
SUBTARGETS:=generic nand tiny

FEATURES:=ramdisk source-only

KERNEL_PATCHVER:=4.14

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += \
	kmod-gpio-button-hotplug swconfig \
	kmod-ath9k wpad-basic uboot-envtools

$(eval $(call BuildTarget))
