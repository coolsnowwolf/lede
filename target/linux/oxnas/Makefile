include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=oxnas
BOARDNAME:=PLXTECH/Oxford NAS782x/OX8xx
SUBTARGETS:=ox810se ox820
FEATURES:=gpio ramdisk rtc squashfs
DEVICE_TYPE:=nas

KERNEL_PATCHVER:=5.4

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += \
	kmod-button-hotplug kmod-input-gpio-keys-polled \
	kmod-ledtrig-timer kmod-leds-gpio uboot-envtools

KERNELNAME:=zImage dtbs

$(eval $(call BuildTarget))
