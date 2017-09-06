#
# Copyright (C) 2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define KernelPackage/ledtrig-adm5120-switch
  SUBMENU:=$(OTHER_MENU)
  TITLE:=LED ADM5120 Switch Port Status Trigger
  DEPENDS:=@TARGET_adm5120
  KCONFIG:=CONFIG_LEDS_TRIGGER_ADM5120_SWITCH
  FILES:=$(LINUX_DIR)/drivers/leds/ledtrig-adm5120-switch.ko
  AUTOLOAD:=$(call AutoLoad,50,ledtrig-adm5120-switch)
endef

define KernelPackage/ledtrig-adm5120-switch/description
 Kernel module to allow LEDs to be controlled by the port states
 of the ADM5120 built-in ethernet switch.
endef

$(eval $(call KernelPackage,ledtrig-adm5120-switch))


define KernelPackage/pata-rb153-cf
  SUBMENU:=$(BLOCK_MENU)
  TITLE:=RouterBOARD 153 CF Slot support
  DEPENDS:=@TARGET_adm5120_rb1xx
  KCONFIG:=CONFIG_PATA_RB153_CF
  FILES:=$(LINUX_DIR)/drivers/ata/pata_rb153_cf.ko
  AUTOLOAD:=$(call AutoLoad,30,pata_rb153_cf,1)
  $(call AddDepends/ata)
endef

define KernelPackage/pata-rb153-cf/description
  Kernel support for the RouterBoard 153 CF slot.
endef

$(eval $(call KernelPackage,pata-rb153-cf,1))


define KernelPackage/usb-adm5120
  SUBMENU:=$(USB_MENU)
  TITLE:=Support for the ADM5120 HCD controller
  DEPENDS:=@TARGET_adm5120
  KCONFIG:=CONFIG_USB_ADM5120_HCD
  FILES:=$(LINUX_DIR)/drivers/usb/host/adm5120-hcd.ko
  AUTOLOAD:=$(call AutoLoad,50,adm5120-hcd,1)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-adm5120/description
 Kernel support for the ADM5120 HCD USB controller
endef

$(eval $(call KernelPackage,usb-adm5120))
