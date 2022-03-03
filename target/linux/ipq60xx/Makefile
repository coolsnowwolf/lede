include $(TOPDIR)/rules.mk

ARCH:=aarch64
BOARD:=ipq60xx
BOARDNAME:=Qualcomm Atheros IPQ60xx
FEATURES:=squashfs ramdisk fpu nand source-only
KERNELNAME:=Image dtbs
CPU_TYPE:=cortex-a53+crypto
SUBTARGETS:=generic

KERNEL_PATCHVER:=5.10

include $(INCLUDE_DIR)/target.mk
DEFAULT_PACKAGES += \
	kmod-leds-gpio kmod-gpio-button-hotplug 

$(eval $(call BuildTarget))
