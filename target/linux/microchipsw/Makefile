include $(TOPDIR)/rules.mk

ARCH:=aarch64
BOARD:=microchipsw
BOARDNAME:=Microchip switches
FEATURES:=squashfs ramdisk fpu rtc emmc
KERNELNAME:=Image
CPU_TYPE:=cortex-a53
SUBTARGETS:=lan969x

KERNEL_PATCHVER:=6.18
KERNEL_TESTING_PATCHVER:=6.18

include $(INCLUDE_DIR)/target.mk
DEFAULT_PACKAGES += \
	kmod-gpio-button-hotplug \
	kmod-leds-gpio \
	uboot-envtools \
	ethtool-full

$(eval $(call BuildTarget))
