#
# Copyright (C) 2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define KernelPackage/pcmcia-bcm63xx
  SUBMENU:=$(PCMCIA_MENU)
  TITLE:=Broadcom BCM63xx PCMCIA support
  DEPENDS:=@TARGET_brcm63xx +kmod-pcmcia-rsrc
  KCONFIG:=CONFIG_PCMCIA_BCM63XX
  FILES:=$(LINUX_DIR)/drivers/pcmcia/bcm63xx_pcmcia.ko
  AUTOLOAD:=$(call AutoLoad,41,bcm63xx_pcmcia)
endef

define KernelPackage/pcmcia-bcm63xx/description
  Kernel support for PCMCIA/CardBus controller on the BCM63xx SoC
endef

$(eval $(call KernelPackage,pcmcia-bcm63xx))

define KernelPackage/bcm63xx-udc
  SUBMENU:=$(USB_MENU)
  TITLE:=Broadcom BCM63xx UDC support
  DEPENDS:=@TARGET_brcm63xx +kmod-usb-gadget
  KCONFIG:=CONFIG_USB_BCM63XX_UDC
  FILES:= \
	$(LINUX_DIR)/drivers/usb/gadget/udc/bcm63xx_udc.ko
  AUTOLOAD:=$(call AutoLoad,51,bcm63xx_udc)
  $(call AddDepends/usb)
endef

define KernelPackage/bcm63xx-udc/description
  Kernel support for the USB gadget (device) controller on the BCM63xx SoC
endef

$(eval $(call KernelPackage,bcm63xx-udc))
