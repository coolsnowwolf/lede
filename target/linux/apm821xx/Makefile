#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=powerpc
BOARD:=apm821xx
BOARDNAME:=AppliedMicro APM821xx
CPU_TYPE:=464fp
FEATURES:=fpu dt gpio
SUBTARGETS:=nand sata

KERNEL_PATCHVER:=5.4

define Target/Description
	Build images for AppliedMicro APM821xx based boards.
endef

include $(INCLUDE_DIR)/target.mk

KERNELNAME:=uImage

DEFAULT_PACKAGES += \
	kmod-leds-gpio kmod-i2c-core kmod-gpio-button-hotplug

$(eval $(call BuildTarget))
