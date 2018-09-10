#
# Copyright (C) 2007-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

BOARD:=adm5120
BOARDNAME:=Infineon/ADMtek ADM5120
FEATURES:=low_mem small_flash source-only
SUBTARGETS:=router_le router_be rb1xx
INITRAMFS_EXTRA_FILES:=

KERNEL_PATCHVER:=3.18

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += admswconfig wpad-mini kmod-input-core \
		    kmod-input-polldev kmod-input-gpio-keys-polled kmod-button-hotplug \
		    kmod-leds-gpio kmod-ledtrig-adm5120-switch

$(eval $(call BuildTarget))
