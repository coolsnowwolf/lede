#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define KernelPackage/leds-nu801
  SUBMENU:=$(LEDS_MENU)
  TITLE:=Meraki MR18 LED support
  DEPENDS:=@TARGET_ar71xx
  KCONFIG:=CONFIG_LEDS_NU801
  FILES:=$(LINUX_DIR)/drivers/leds/leds-nu801.ko
  AUTOLOAD:=$(call AutoLoad,60,leds-nu801)
endef

define KernelPackage/leds-nu801/description
 Kernel module for the nu801 LED driver used on the Meraki MR18.
endef

$(eval $(call KernelPackage,leds-nu801))


define KernelPackage/leds-rb750
  SUBMENU:=$(LEDS_MENU)
  TITLE:=RouterBOARD 750 LED support
  DEPENDS:=@TARGET_ar71xx
  KCONFIG:=CONFIG_LEDS_RB750
  FILES:=$(LINUX_DIR)/drivers/leds/leds-rb750.ko
  AUTOLOAD:=$(call AutoLoad,60,leds-rb750)
endef

define KernelPackage/leds-rb750/description
 Kernel module for the LEDs on the MikroTik RouterBOARD 750.
endef

$(eval $(call KernelPackage,leds-rb750))


define KernelPackage/leds-wndr3700-usb
  SUBMENU:=$(LEDS_MENU)
  TITLE:=WNDR3700 USB LED support
  DEPENDS:=@TARGET_ar71xx
  KCONFIG:=CONFIG_LEDS_WNDR3700_USB
  FILES:=$(LINUX_DIR)/drivers/leds/leds-wndr3700-usb.ko
  AUTOLOAD:=$(call AutoLoad,60,leds-wndr3700-usb)
endef

define KernelPackage/leds-wndr3700-usb/description
 Kernel module for the USB LED on the NETGEAR WNDR3700 board.
endef

$(eval $(call KernelPackage,leds-wndr3700-usb))


define KernelPackage/spi-vsc7385
  SUBMENU:=$(SPI_MENU)
  TITLE:=Vitesse VSC7385 ethernet switch driver
  DEPENDS:=@TARGET_ar71xx
  KCONFIG:=CONFIG_SPI_VSC7385
  FILES:=$(LINUX_DIR)/drivers/spi/spi-vsc7385.ko
  AUTOLOAD:=$(call AutoLoad,93,spi-vsc7385)
endef

define KernelPackage/spi-vsc7385/description
  This package contains the SPI driver for the Vitesse VSC7385 ethernet switch.
endef

$(eval $(call KernelPackage,spi-vsc7385))
