include $(TOPDIR)/rules.mk

ARCH:=aarch64
BOARD:=qualcommax
BOARDNAME:=Qualcomm Atheros 802.11ax WiSoC-s
FEATURES:=squashfs ramdisk fpu nand rtc emmc
KERNELNAME:=Image dtbs
CPU_TYPE:=cortex-a53
SUBTARGETS:=ipq807x

KERNEL_PATCHVER:=5.10
KERNEL_TESTING_PATCHVER:=5.15

include $(INCLUDE_DIR)/target.mk
DEFAULT_PACKAGES += \
	kmod-usb3 kmod-usb-dwc3 kmod-usb-dwc3-qcom \
	kmod-leds-gpio kmod-gpio-button-hotplug \
	ath11k-firmware-ipq8074 kmod-ath11k-ahb \
	autocore-arm htop wpad-openssl uboot-envtools e2fsprogs kmod-fs-ext4 losetup \
	kmod-qca-nss-dp kmod-qca-nss-drv-64 \
	kmod-qca-nss-drv-pppoe-64 kmod-qca-nss-ecm-64 \
	kmod-qca-nss-drv-bridge-mgr-64 kmod-qca-nss-drv-vlan-mgr-64 \
	nss-firmware-ipq8074

$(eval $(call BuildTarget))
