include $(TOPDIR)/rules.mk

ARCH:=aarch64
BOARD:=qualcommbe
BOARDNAME:=Qualcomm Atheros 802.11be WiSoC-s
FEATURES:=squashfs ramdisk fpu nand rtc emmc source-only
KERNELNAME:=Image
CPU_TYPE:=cortex-a53
SUBTARGETS:=ipq95xx

KERNEL_PATCHVER:=6.12

include $(INCLUDE_DIR)/target.mk
DEFAULT_PACKAGES += \
	kmod-usb3 kmod-usb-dwc3 kmod-usb-dwc3-qcom \
	kmod-leds-gpio kmod-gpio-button-hotplug \
	wpad-basic-mbedtls uboot-envtools \
	e2fsprogs kmod-fs-ext4 losetup

$(eval $(call BuildTarget))
