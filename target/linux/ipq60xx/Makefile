include $(TOPDIR)/rules.mk

ARCH:=aarch64
BOARD:=ipq60xx
BOARDNAME:=Qualcomm Atheros IPQ60xx
FEATURES:=squashfs ramdisk fpu nand rtc emmc
KERNELNAME:=Image dtbs
CPU_TYPE:=cortex-a53
SUBTARGETS:=generic

KERNEL_PATCHVER:=5.15

include $(INCLUDE_DIR)/target.mk
DEFAULT_PACKAGES += \
	kmod-usb3 kmod-usb-dwc3 kmod-usb-dwc3-qcom \
	kmod-leds-gpio kmod-gpio-button-hotplug \
	uboot-envtools \
	kmod-qca-nss-dp kmod-qca-nss-drv-64 kmod-qca-nss-drv-pppoe-64 \
	nss-firmware-ipq6018 qca-nss-ecm-64

$(eval $(call BuildTarget))
