#
# Copyright (C) 2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define KernelPackage/ata-magicbox-cf
  SUBMENU:=$(BLOCK_MENU)
  TITLE:=Magicbox v2/OpenRB Compact flash support
  DEPENDS:=@TARGET_ppc40x
  KCONFIG:=CONFIG_PATA_MAGICBOX_CF
  FILES:=$(LINUX_DIR)/drivers/ata/pata_magicbox_cf.ko
  AUTOLOAD:=$(call AutoLoad,41,pata_magicbox_cf,1)
  $(call AddDepends/ata)
endef

define KernelPackage/ata-magicbox-cf/description
  Support for Magicbox v2/OpenRB on-board CF slot.
endef

$(eval $(call KernelPackage,ata-magicbox-cf))


define KernelPackage/usb-isp116x-hcd
  TITLE:=Support for the ISP116x USB Host Controller
  DEPENDS:=@TARGET_ppc40x
  KCONFIG:= \
	CONFIG_USB_ISP116X_HCD \
	CONFIG_USB_ISP116X_HCD_OF=y \
	CONFIG_USB_ISP116X_HCD_PLATFORM=n
  FILES:=$(LINUX_DIR)/drivers/usb/host/isp116x-hcd.ko
  AUTOLOAD:=$(call AutoLoad,50,isp116x-hcd)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-isp116x-hcd/description
  Kernel support for the ISP116X USB Host Controller
endef

$(eval $(call KernelPackage,usb-isp116x-hcd))
