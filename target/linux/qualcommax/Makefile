include $(TOPDIR)/rules.mk

ARCH:=aarch64
BOARD:=qualcommax
BOARDNAME:=Qualcomm Atheros 802.11ax WiSoC-s
FEATURES:=squashfs ramdisk fpu nand rtc emmc
KERNELNAME:=Image dtbs
CPU_TYPE:=cortex-a53
SUBTARGETS:=ipq50xx ipq60xx ipq807x

KERNEL_PATCHVER:=6.1
KERNEL_TESTING_PATCHVER:=6.6

include $(INCLUDE_DIR)/target.mk
DEFAULT_PACKAGES += \
	kmod-usb3 kmod-usb-dwc3 kmod-usb-dwc3-qcom \
	kmod-leds-gpio kmod-gpio-button-hotplug \
	kmod-qca-nss-dp kmod-qca-nss-drv kmod-qca-ssdk \
	kmod-qca-nss-ecm kmod-qca-nss-drv-bridge-mgr \
	kmod-qca-nss-drv-vlan kmod-qca-nss-drv-pppoe \
	kmod-ath11k-ahb wpad-openssl uboot-envtools \
	e2fsprogs f2fsck mkf2fs losetup automount

$(eval $(call BuildTarget))
