# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2019 OpenWrt.org

define KernelPackage/rp1-adc
  SUBMENU:=$(OTHER_MENU)
  TITLE:=RP1 ADC and temperature sensor driver
  KCONFIG:=CONFIG_SENSORS_RP1_ADC
  FILES:=$(LINUX_DIR)/drivers/hwmon/rp1-adc.ko
  AUTOLOAD:=$(call AutoLoad,21,rp1-adc)
  DEPENDS:=@TARGET_bcm27xx_bcm2712
endef

define KernelPackage/rp1-adc/description
  Kernel module for RP1 silicon providing ADC and
  temperature monitoring.
endef

$(eval $(call KernelPackage,rp1-adc))


define KernelPackage/hwmon-raspberrypi
  TITLE:=Raspberry Pi voltage monitor
  KCONFIG:=CONFIG_SENSORS_RASPBERRYPI_HWMON
  FILES:=$(LINUX_DIR)/drivers/hwmon/raspberrypi-hwmon.ko
  AUTOLOAD:=$(call AutoLoad,60,raspberrypi-hwmon)
  $(call AddDepends/hwmon,@TARGET_bcm27xx)
endef

define KernelPackage/hwmon-raspberrypi/description
  Kernel module for voltage sensor on the Raspberry Pi
endef

$(eval $(call KernelPackage,hwmon-raspberrypi))
