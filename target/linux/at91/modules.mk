#
# Copyright (C) 2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define KernelPackage/mmc-at91
  SUBMENU:=$(OTHER_MENU)
  TITLE:=MMC/SD Card Support on AT91
  DEPENDS:=@TARGET_at91 +kmod-mmc
  KCONFIG:=CONFIG_MMC_AT91
  FILES:=$(LINUX_DIR)/drivers/mmc/host/at91_mci.ko
  AUTOLOAD:=$(call AutoLoad,90,at91_mci,1)
endef

define KernelPackage/mmc-at91/description
 Kernel support for MMC/SD cards on the AT91 target
endef

$(eval $(call KernelPackage,mmc-at91))

define KernelPackage/pwm-atmel
  SUBMENU:=$(OTHER_MENU)
  TITLE:=PWM on atmel SoC
  DEPENDS:=@TARGET_at91
  KCONFIG:=CONFIG_ATMEL_PWM
  FILES:=$(LINUX_DIR)/drivers/misc/atmel_pwm.ko
  AUTOLOAD:=$(call AutoLoad,51,atmel_pwm)
endef

define KernelPackage/pwm-atmel/description
 Kernel module to use the PWM channel on ATMEL SoC
endef

$(eval $(call KernelPackage,pwm-atmel))

define KernelPackage/at91-adc
  SUBMENU:=$(OTHER_MENU)
  TITLE:=ADC on atmel SoC
  DEPENDS:=@TARGET_at91 +kmod-iio-core +kmod-input-core
  KCONFIG:=CONFIG_AT91_ADC
  FILES:=$(LINUX_DIR)/drivers/iio/adc/at91_adc.ko
  AUTOLOAD:=$(call AutoLoad,40,at91_adc)
endef

define KernelPackage/at91-adc/description
 Kernel module to use the ADC channels of SoC
endef

$(eval $(call KernelPackage,at91-adc))

define KernelPackage/at91-udc
  SUBMENU:=$(USB_MENU)
  TITLE:=USB Device Controller on atmel SoC
  DEPENDS:=@TARGET_at91 +kmod-usb-gadget +kmod-regmap-core
  KCONFIG:=CONFIG_USB_AT91
ifneq ($(wildcard $(LINUX_DIR)/drivers/usb/gadget/udc/at91_udc.ko),)
  FILES:=$(LINUX_DIR)/drivers/usb/gadget/udc/at91_udc.ko
else
  FILES:=$(LINUX_DIR)/drivers/usb/gadget/at91_udc.ko
endif
  AUTOLOAD:=$(call AutoLoad,51,at91_udc)
endef

define KernelPackage/at91-udc/description
 Kernel module to use the USB Device controller for Atmel AT91
endef

$(eval $(call KernelPackage,at91-udc))

define KernelPackage/atmel-usba-udc
  SUBMENU:=$(USB_MENU)
  TITLE:=High-speed USB Device Controller on atmel SoC
  DEPENDS:=@TARGET_at91 +kmod-usb-gadget
  KCONFIG:=CONFIG_USB_ATMEL_USBA
ifneq ($(wildcard $(LINUX_DIR)/drivers/usb/gadget/udc/atmel_usba_udc.ko),)
  FILES:=$(LINUX_DIR)/drivers/usb/gadget/udc/atmel_usba_udc.ko
else
  FILES:=$(LINUX_DIR)/drivers/usb/gadget/atmel_usba_udc.ko
endif
  AUTOLOAD:=$(call AutoLoad,51,atmel_usba_udc)
endef

define KernelPackage/atmel-usba-udc/description
 Kernel module to use the High-speed USB Device controller for Atmel AT91
endef

$(eval $(call KernelPackage,atmel-usba-udc))

I2C_AT91_MODULES:=\
  CONFIG_I2C_AT91:drivers/i2c/busses/i2c-at91

define KernelPackage/at91-i2c
  $(call i2c_defaults,$(I2C_AT91_MODULES),55)
  TITLE:=I2C (TWI) master driver for Atmel AT91
  DEPENDS:=@TARGET_at91 +kmod-i2c-core
endef

define KernelPackage/at91-i2c/description
 Kernel module to use the I2C (TWI) master driver for Atmel AT91
endef

$(eval $(call KernelPackage,at91-i2c))

