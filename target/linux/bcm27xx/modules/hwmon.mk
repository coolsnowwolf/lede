#
# Copyright (C) 2019 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

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


define KernelPackage/hwmon-rpi-poe-fan
  SUBMENU:=$(HWMON_MENU)
  TITLE:=Raspberry Pi PoE HAT fan
  DEPENDS:=@TARGET_bcm27xx +kmod-hwmon-core
  KCONFIG:=CONFIG_SENSORS_RPI_POE_FAN
  FILES:=$(LINUX_DIR)/drivers/hwmon/rpi-poe-fan.ko
  AUTOLOAD:=$(call AutoProbe,rpi-poe-fan)
endef

define KernelPackage/hwmon-rpi-poe-fan/description
  Raspberry Pi PoE HAT fan driver
endef

$(eval $(call KernelPackage,hwmon-rpi-poe-fan))
